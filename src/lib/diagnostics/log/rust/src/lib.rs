// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

//! Publish diagnostics as a stream of log messages.

use fidl_fuchsia_diagnostics_stream::Record;
use fidl_fuchsia_logger::LogSinkMarker;
use fuchsia_component::client::connect_to_service;
use fuchsia_zircon::{self as zx};
use std::{any::TypeId, fmt::Debug, future::Future};
use thiserror::Error;
use tracing::{
    span::{Attributes, Id, Record as TracingRecord},
    subscriber::{Interest, Subscriber},
    Event, Metadata,
};
use tracing_core::span::Current;
use tracing_log::LogTracer;
use tracing_subscriber::{layer::Layered, prelude::*, registry::Registry};

mod filter;
mod sink;

use filter::InterestFilter;
use sink::Sink;

/// Calls [init_publishing`] and spawns the interest listener future as a `fuchsia_async::Task`.
///
/// Implemented as a macro to avoid a direct dependency on `fuchsia_async`.
///
/// # Panics
///
/// If [`init_publishing`] returns an error.
#[macro_export]
macro_rules! init {
    () => {
        fuchsia_async::Task::spawn($crate::init_publishing().unwrap()).detach()
    };
}

/// Creates a publisher and installs it as the global default.
///
/// Also installs a bridge to capture events from the `log` crate's macros and a panic hook to
/// ensure panic messages are logged when stderr is not hooked up to anything.
#[must_use = "Interest registration future must be polled."]
pub fn init_publishing() -> Result<impl Future<Output = ()>, PublishError> {
    let (publisher, on_interest) = Publisher::new()?;

    tracing::subscriber::set_global_default(publisher)?;
    LogTracer::init()?;

    let previous_hook = std::panic::take_hook();
    std::panic::set_hook(Box::new(move |info| {
        tracing::error!({ %info }, "PANIC");
        previous_hook(info);
    }));

    tracing::debug!("Logging initialized");
    Ok(on_interest)
}

/// A `Publisher` acts as broker, implementing [`tracing::Subscriber`] to receive diagnostic
/// events from a component, and then forwarding that data on to a diagnostics service.
pub struct Publisher {
    inner: Layered<InterestFilter, Layered<Sink, Registry>>,
}

impl Publisher {
    /// Construct a new `Publisher` from the provided `LogSink`. Returns a `Publisher` and a future
    /// which must be polled to listen to interest/severity changes from the environment `LogSink`.
    fn new() -> Result<(Self, impl Future<Output = ()>), PublishError> {
        let log_sink = connect_to_service::<LogSinkMarker>()
            .map_err(|e| e.to_string())
            .map_err(PublishError::LogSinkConnect)?;
        let sink_events = log_sink.take_event_stream();
        let publisher = Sink::new(log_sink)?;
        let (filter, on_change) = InterestFilter::new(sink_events);
        Ok((Self { inner: Registry::default().with(publisher).with(filter) }, on_change))
    }

    // TODO(fxbug.dev/71242) delete this and make Publisher private
    /// Publish the provided event for testing.
    pub fn event_for_testing(&self, file: &str, line: u32, record: Record) {
        // TODO(fxbug.dev/62858) uncomment
        // let filter: &InterestFilter = (&self.inner as &dyn Subscriber).downcast_ref().unwrap();
        // if filter.enabled_for_testing(file, line, &record) {
        let sink: &Sink = (&self.inner as &dyn Subscriber).downcast_ref().unwrap();
        sink.event_for_testing(file, line, record);
        // }
    }
}

impl Subscriber for Publisher {
    fn enabled(&self, metadata: &Metadata<'_>) -> bool {
        self.inner.enabled(metadata)
    }
    fn new_span(&self, span: &Attributes<'_>) -> Id {
        self.inner.new_span(span)
    }
    fn record(&self, span: &Id, values: &TracingRecord<'_>) {
        self.inner.record(span, values)
    }
    fn record_follows_from(&self, span: &Id, follows: &Id) {
        self.inner.record_follows_from(span, follows)
    }
    fn event(&self, event: &Event<'_>) {
        self.inner.event(event)
    }
    fn enter(&self, span: &Id) {
        self.inner.enter(span)
    }
    fn exit(&self, span: &Id) {
        self.inner.exit(span)
    }
    fn register_callsite(&self, metadata: &'static Metadata<'static>) -> Interest {
        self.inner.register_callsite(metadata)
    }
    fn clone_span(&self, id: &Id) -> Id {
        self.inner.clone_span(id)
    }
    fn try_close(&self, id: Id) -> bool {
        self.inner.try_close(id)
    }
    fn current_span(&self) -> Current {
        self.inner.current_span()
    }
    unsafe fn downcast_raw(&self, id: TypeId) -> Option<*const ()> {
        if id == TypeId::of::<Self>() {
            Some(self as *const Self as *const ())
        } else {
            None
        }
    }
}

/// Errors arising while forwarding a diagnostics stream to the environment.
#[derive(Debug, Error)]
pub enum PublishError {
    /// Connection to fuchsia.logger.LogSink failed.
    #[error("failed to connect to fuchsia.logger.LogSink ({0})")]
    LogSinkConnect(String),

    /// Couldn't create a new socket.
    #[error("failed to create a socket for logging")]
    MakeSocket(#[source] zx::Status),

    /// An issue with the LogSink channel or socket prevented us from sending it to the `LogSink`.
    #[error("failed to send a socket to the LogSink")]
    SendSocket(#[source] fidl::Error),

    /// Setting the default global [`tracing::Subscriber`] failed.
    #[error("failed to install forwarder as the global default")]
    SetGlobalDefault(#[from] tracing_core::dispatcher::SetGlobalDefaultError),

    /// Installing a forwarder from [`log`] macros to [`tracing`] macros failed.
    #[error("failed to install a forwarder from `log` to `tracing`")]
    InitLogForward(#[from] tracing_log::log_tracer::SetLoggerError),
}
