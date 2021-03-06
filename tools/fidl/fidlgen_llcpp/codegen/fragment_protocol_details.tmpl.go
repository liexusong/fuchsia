// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package codegen

// fragmentProtocolDetailsTmpl contains the definition for
// fidl::internal::ProtocolDetails<Protocol>.
const fragmentProtocolDetailsTmpl = `
{{- define "ProtocolDetailsDeclaration" }}
{{ EnsureNamespace "" }}
{{- IfdefFuchsia }}
template<>
struct {{ .ProtocolDetails }} {
  {{- if .DiscoverableName }}
    static constexpr char DiscoverableName[] = {{ .DiscoverableName }};
  {{- end }}
};
{{- EndifFuchsia }}
{{- end }}
`
