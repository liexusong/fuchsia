// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io' show File;

import 'package:mockito/mockito.dart';
import 'package:test/test.dart';

import 'package:sl4f/sl4f.dart';

class MockSl4f extends Mock implements Sl4f {}

class MockDump extends Mock implements Dump {}

class RunProcessorObserver {
  void runTraceProcessor(String processor, List<String> args) {}
}

class MockRunProcessorObserver extends Mock implements RunProcessorObserver {}

class FakePerformanceTools extends Performance {
  final RunProcessorObserver _observer;
  FakePerformanceTools(Sl4f _sl4f, Dump _dump, RunProcessorObserver observer)
      : _observer = observer,
        super(_sl4f, _dump);

  @override
  Future<bool> runTraceProcessor(String processor, List<String> args) async {
    _observer.runTraceProcessor(processor, args);
    return true;
  }
}

const String sl4fTraceRequestMethod = 'Performance_facade.GetTraceFile';

void main(List<String> args) {
  // Verifies that [Performance.downloadTraceFile] requests a file from Sl4f and
  // writes its contents to a local file.
  test('download trace requests a file and writes it', () async {
    final List<int> traceData =
        utf8.encode('{"traceEvents": [], "displayTimeUnit": "ns"}');
    final mockDump = MockDump();
    final mockSl4f = MockSl4f();

    when(mockSl4f.request(sl4fTraceRequestMethod, any))
        .thenAnswer((_) => Future.value(base64.encode(traceData)));

    final performance = Performance(mockSl4f, mockDump);

    await performance.downloadTraceFile('test-trace');

    // Verify Sl4f.request.
    final verifyMockSl4fRequest =
        verify(mockSl4f.request(sl4fTraceRequestMethod, captureAny))..called(1);
    expect(verifyMockSl4fRequest.captured.single,
        {'path': '/tmp/test-trace-trace.json'});

    // Verify Dump.writeAsBytes.
    final verifyMockDumpWriteAsBytes =
        verify(mockDump.writeAsBytes(captureAny, captureAny, captureAny))
          ..called(1);
    expect(verifyMockDumpWriteAsBytes.captured,
        ['test-trace-trace', 'json', traceData]);
  });

  test('download trace handles non-utf8 data', () async {
    final List<int> nonUtf8Data = [
      0x1f,
      0x8b,
      0x08,
      0x08,
      0x35,
      0xdc,
      0x00,
      0x03
    ];

    expect(() => utf8.decode(nonUtf8Data), throwsException);

    final mockDump = MockDump();
    final mockSl4f = MockSl4f();

    when(mockSl4f.request(sl4fTraceRequestMethod, any))
        .thenAnswer((_) => Future.value(base64.encode(nonUtf8Data)));

    final performance = Performance(mockSl4f, mockDump);

    await performance.downloadTraceFile('non-utf8');

    final verifyMockSl4fRequest =
        verify(mockSl4f.request(sl4fTraceRequestMethod, captureAny))..called(1);
    expect(verifyMockSl4fRequest.captured.single,
        {'path': '/tmp/non-utf8-trace.json'});

    final verifyMockDumpWriteAsBytes =
        verify(mockDump.writeAsBytes(captureAny, captureAny, captureAny))
          ..called(1);
    expect(verifyMockDumpWriteAsBytes.captured,
        ['non-utf8-trace', 'json', nonUtf8Data]);
  });

  test('process trace', () async {
    final mockDump = MockDump();
    final mockSl4f = MockSl4f();
    final mockRunProcessorObserver = MockRunProcessorObserver();
    final performance =
        FakePerformanceTools(mockSl4f, mockDump, mockRunProcessorObserver);

    // Test trace processing with [appName] set.
    await performance.processTrace(
        '/bin/process_sample_trace', File('sample-trace.json'), 'test1',
        appName: 'test-app');

    var verifyMockRunProcessObserver = verify(mockRunProcessorObserver
        .runTraceProcessor('/bin/process_sample_trace', captureAny))
      ..called(1);
    var capturedArgs = verifyMockRunProcessObserver.captured.single;
    expect(capturedArgs[0], '-test_suite_name=test1');
    expect(capturedArgs[1], '-flutter_app_name=test-app');
    expect(capturedArgs[2], endsWith('test1-benchmark.json'));
    expect(capturedArgs[3], endsWith('sample-trace.json'));

    // Test trace processing without an [appName] set.
    await performance.processTrace(
        '/bin/process_sample_trace', File('sample-trace.json'), 'test1');

    verifyMockRunProcessObserver = verify(mockRunProcessorObserver
        .runTraceProcessor('/bin/process_sample_trace', captureAny))
      ..called(1);
    capturedArgs = verifyMockRunProcessObserver.captured.single;
    expect(capturedArgs[0], '-test_suite_name=test1');
    expect(capturedArgs[1], endsWith('test1-benchmark.json'));
    expect(capturedArgs[2], endsWith('sample-trace.json'));
  });
}
