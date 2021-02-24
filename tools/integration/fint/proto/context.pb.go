// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.25.0
// 	protoc        v3.8.0
// source: context.proto

package proto

import (
	proto "github.com/golang/protobuf/proto"
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

// This is a compile-time assertion that a sufficiently up-to-date version
// of the legacy proto package is being used.
const _ = proto.ProtoPackageIsVersion4

// Context contains all of the dynamic configuration values for building
// Fuchsia. These values are context-dependent in that they vary based on git
// history and local filesystem layout, so they can only be known at runtime and
// cannot come from data checked into version control.
type Context struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// Absolute path to the root of the Fuchsia checkout.
	CheckoutDir string `protobuf:"bytes,1,opt,name=checkout_dir,json=checkoutDir,proto3" json:"checkout_dir,omitempty"`
	// Absolute path to the build output directory.
	BuildDir string `protobuf:"bytes,2,opt,name=build_dir,json=buildDir,proto3" json:"build_dir,omitempty"`
	// Absolute path to a directory where build artifacts intended to be
	// exposed to the caller should be written.
	ArtifactDir string `protobuf:"bytes,3,opt,name=artifact_dir,json=artifactDir,proto3" json:"artifact_dir,omitempty"`
	// Files changed in the commit being tested.
	ChangedFiles []*Context_ChangedFile `protobuf:"bytes,4,rep,name=changed_files,json=changedFiles,proto3" json:"changed_files,omitempty"`
	// Passed through to the `sdk_id` gn arg.
	SdkId string `protobuf:"bytes,5,opt,name=sdk_id,json=sdkId,proto3" json:"sdk_id,omitempty"`
	// Cache directory that's persisted between builds.
	CacheDir string `protobuf:"bytes,6,opt,name=cache_dir,json=cacheDir,proto3" json:"cache_dir,omitempty"`
	// Passed through to the `build_info_version` gn arg.
	ReleaseVersion string `protobuf:"bytes,7,opt,name=release_version,json=releaseVersion,proto3" json:"release_version,omitempty"`
	// Paths to directories that contain alternate versions of these toolchains.
	// For each one, if set, we'll use the toolchain version at the specified path
	// instead of the version included in the checkout.
	ClangToolchainDir string `protobuf:"bytes,8,opt,name=clang_toolchain_dir,json=clangToolchainDir,proto3" json:"clang_toolchain_dir,omitempty"`
	GccToolchainDir   string `protobuf:"bytes,9,opt,name=gcc_toolchain_dir,json=gccToolchainDir,proto3" json:"gcc_toolchain_dir,omitempty"`
	RustToolchainDir  string `protobuf:"bytes,10,opt,name=rust_toolchain_dir,json=rustToolchainDir,proto3" json:"rust_toolchain_dir,omitempty"`
	// Whether to build for the purposes of collecting coverage.
	CollectCoverage bool `protobuf:"varint,11,opt,name=collect_coverage,json=collectCoverage,proto3" json:"collect_coverage,omitempty"`
	// Use this many jobs if building with Goma (ignored otherwise).
	GomaJobCount int32 `protobuf:"varint,12,opt,name=goma_job_count,json=gomaJobCount,proto3" json:"goma_job_count,omitempty"`
}

func (x *Context) Reset() {
	*x = Context{}
	if protoimpl.UnsafeEnabled {
		mi := &file_context_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Context) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Context) ProtoMessage() {}

func (x *Context) ProtoReflect() protoreflect.Message {
	mi := &file_context_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Context.ProtoReflect.Descriptor instead.
func (*Context) Descriptor() ([]byte, []int) {
	return file_context_proto_rawDescGZIP(), []int{0}
}

func (x *Context) GetCheckoutDir() string {
	if x != nil {
		return x.CheckoutDir
	}
	return ""
}

func (x *Context) GetBuildDir() string {
	if x != nil {
		return x.BuildDir
	}
	return ""
}

func (x *Context) GetArtifactDir() string {
	if x != nil {
		return x.ArtifactDir
	}
	return ""
}

func (x *Context) GetChangedFiles() []*Context_ChangedFile {
	if x != nil {
		return x.ChangedFiles
	}
	return nil
}

func (x *Context) GetSdkId() string {
	if x != nil {
		return x.SdkId
	}
	return ""
}

func (x *Context) GetCacheDir() string {
	if x != nil {
		return x.CacheDir
	}
	return ""
}

func (x *Context) GetReleaseVersion() string {
	if x != nil {
		return x.ReleaseVersion
	}
	return ""
}

func (x *Context) GetClangToolchainDir() string {
	if x != nil {
		return x.ClangToolchainDir
	}
	return ""
}

func (x *Context) GetGccToolchainDir() string {
	if x != nil {
		return x.GccToolchainDir
	}
	return ""
}

func (x *Context) GetRustToolchainDir() string {
	if x != nil {
		return x.RustToolchainDir
	}
	return ""
}

func (x *Context) GetCollectCoverage() bool {
	if x != nil {
		return x.CollectCoverage
	}
	return false
}

func (x *Context) GetGomaJobCount() int32 {
	if x != nil {
		return x.GomaJobCount
	}
	return 0
}

type Context_ChangedFile struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// Relative path to the file within the checkout.
	Path string `protobuf:"bytes,1,opt,name=path,proto3" json:"path,omitempty"`
}

func (x *Context_ChangedFile) Reset() {
	*x = Context_ChangedFile{}
	if protoimpl.UnsafeEnabled {
		mi := &file_context_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Context_ChangedFile) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Context_ChangedFile) ProtoMessage() {}

func (x *Context_ChangedFile) ProtoReflect() protoreflect.Message {
	mi := &file_context_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Context_ChangedFile.ProtoReflect.Descriptor instead.
func (*Context_ChangedFile) Descriptor() ([]byte, []int) {
	return file_context_proto_rawDescGZIP(), []int{0, 0}
}

func (x *Context_ChangedFile) GetPath() string {
	if x != nil {
		return x.Path
	}
	return ""
}

var File_context_proto protoreflect.FileDescriptor

var file_context_proto_rawDesc = []byte{
	0x0a, 0x0d, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12,
	0x04, 0x66, 0x69, 0x6e, 0x74, 0x22, 0x87, 0x04, 0x0a, 0x07, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x78,
	0x74, 0x12, 0x21, 0x0a, 0x0c, 0x63, 0x68, 0x65, 0x63, 0x6b, 0x6f, 0x75, 0x74, 0x5f, 0x64, 0x69,
	0x72, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0b, 0x63, 0x68, 0x65, 0x63, 0x6b, 0x6f, 0x75,
	0x74, 0x44, 0x69, 0x72, 0x12, 0x1b, 0x0a, 0x09, 0x62, 0x75, 0x69, 0x6c, 0x64, 0x5f, 0x64, 0x69,
	0x72, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x62, 0x75, 0x69, 0x6c, 0x64, 0x44, 0x69,
	0x72, 0x12, 0x21, 0x0a, 0x0c, 0x61, 0x72, 0x74, 0x69, 0x66, 0x61, 0x63, 0x74, 0x5f, 0x64, 0x69,
	0x72, 0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0b, 0x61, 0x72, 0x74, 0x69, 0x66, 0x61, 0x63,
	0x74, 0x44, 0x69, 0x72, 0x12, 0x3e, 0x0a, 0x0d, 0x63, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x64, 0x5f,
	0x66, 0x69, 0x6c, 0x65, 0x73, 0x18, 0x04, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x19, 0x2e, 0x66, 0x69,
	0x6e, 0x74, 0x2e, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x2e, 0x43, 0x68, 0x61, 0x6e, 0x67,
	0x65, 0x64, 0x46, 0x69, 0x6c, 0x65, 0x52, 0x0c, 0x63, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x64, 0x46,
	0x69, 0x6c, 0x65, 0x73, 0x12, 0x15, 0x0a, 0x06, 0x73, 0x64, 0x6b, 0x5f, 0x69, 0x64, 0x18, 0x05,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x05, 0x73, 0x64, 0x6b, 0x49, 0x64, 0x12, 0x1b, 0x0a, 0x09, 0x63,
	0x61, 0x63, 0x68, 0x65, 0x5f, 0x64, 0x69, 0x72, 0x18, 0x06, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08,
	0x63, 0x61, 0x63, 0x68, 0x65, 0x44, 0x69, 0x72, 0x12, 0x27, 0x0a, 0x0f, 0x72, 0x65, 0x6c, 0x65,
	0x61, 0x73, 0x65, 0x5f, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x18, 0x07, 0x20, 0x01, 0x28,
	0x09, 0x52, 0x0e, 0x72, 0x65, 0x6c, 0x65, 0x61, 0x73, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f,
	0x6e, 0x12, 0x2e, 0x0a, 0x13, 0x63, 0x6c, 0x61, 0x6e, 0x67, 0x5f, 0x74, 0x6f, 0x6f, 0x6c, 0x63,
	0x68, 0x61, 0x69, 0x6e, 0x5f, 0x64, 0x69, 0x72, 0x18, 0x08, 0x20, 0x01, 0x28, 0x09, 0x52, 0x11,
	0x63, 0x6c, 0x61, 0x6e, 0x67, 0x54, 0x6f, 0x6f, 0x6c, 0x63, 0x68, 0x61, 0x69, 0x6e, 0x44, 0x69,
	0x72, 0x12, 0x2a, 0x0a, 0x11, 0x67, 0x63, 0x63, 0x5f, 0x74, 0x6f, 0x6f, 0x6c, 0x63, 0x68, 0x61,
	0x69, 0x6e, 0x5f, 0x64, 0x69, 0x72, 0x18, 0x09, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0f, 0x67, 0x63,
	0x63, 0x54, 0x6f, 0x6f, 0x6c, 0x63, 0x68, 0x61, 0x69, 0x6e, 0x44, 0x69, 0x72, 0x12, 0x2c, 0x0a,
	0x12, 0x72, 0x75, 0x73, 0x74, 0x5f, 0x74, 0x6f, 0x6f, 0x6c, 0x63, 0x68, 0x61, 0x69, 0x6e, 0x5f,
	0x64, 0x69, 0x72, 0x18, 0x0a, 0x20, 0x01, 0x28, 0x09, 0x52, 0x10, 0x72, 0x75, 0x73, 0x74, 0x54,
	0x6f, 0x6f, 0x6c, 0x63, 0x68, 0x61, 0x69, 0x6e, 0x44, 0x69, 0x72, 0x12, 0x29, 0x0a, 0x10, 0x63,
	0x6f, 0x6c, 0x6c, 0x65, 0x63, 0x74, 0x5f, 0x63, 0x6f, 0x76, 0x65, 0x72, 0x61, 0x67, 0x65, 0x18,
	0x0b, 0x20, 0x01, 0x28, 0x08, 0x52, 0x0f, 0x63, 0x6f, 0x6c, 0x6c, 0x65, 0x63, 0x74, 0x43, 0x6f,
	0x76, 0x65, 0x72, 0x61, 0x67, 0x65, 0x12, 0x24, 0x0a, 0x0e, 0x67, 0x6f, 0x6d, 0x61, 0x5f, 0x6a,
	0x6f, 0x62, 0x5f, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x18, 0x0c, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0c,
	0x67, 0x6f, 0x6d, 0x61, 0x4a, 0x6f, 0x62, 0x43, 0x6f, 0x75, 0x6e, 0x74, 0x1a, 0x21, 0x0a, 0x0b,
	0x43, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x64, 0x46, 0x69, 0x6c, 0x65, 0x12, 0x12, 0x0a, 0x04, 0x70,
	0x61, 0x74, 0x68, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x70, 0x61, 0x74, 0x68, 0x42,
	0x35, 0x5a, 0x33, 0x67, 0x6f, 0x2e, 0x66, 0x75, 0x63, 0x68, 0x73, 0x69, 0x61, 0x2e, 0x64, 0x65,
	0x76, 0x2f, 0x66, 0x75, 0x63, 0x68, 0x73, 0x69, 0x61, 0x2f, 0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2f,
	0x69, 0x6e, 0x74, 0x65, 0x67, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f, 0x66, 0x69, 0x6e, 0x74,
	0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_context_proto_rawDescOnce sync.Once
	file_context_proto_rawDescData = file_context_proto_rawDesc
)

func file_context_proto_rawDescGZIP() []byte {
	file_context_proto_rawDescOnce.Do(func() {
		file_context_proto_rawDescData = protoimpl.X.CompressGZIP(file_context_proto_rawDescData)
	})
	return file_context_proto_rawDescData
}

var file_context_proto_msgTypes = make([]protoimpl.MessageInfo, 2)
var file_context_proto_goTypes = []interface{}{
	(*Context)(nil),             // 0: fint.Context
	(*Context_ChangedFile)(nil), // 1: fint.Context.ChangedFile
}
var file_context_proto_depIdxs = []int32{
	1, // 0: fint.Context.changed_files:type_name -> fint.Context.ChangedFile
	1, // [1:1] is the sub-list for method output_type
	1, // [1:1] is the sub-list for method input_type
	1, // [1:1] is the sub-list for extension type_name
	1, // [1:1] is the sub-list for extension extendee
	0, // [0:1] is the sub-list for field type_name
}

func init() { file_context_proto_init() }
func file_context_proto_init() {
	if File_context_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_context_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Context); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_context_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Context_ChangedFile); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_context_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   2,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_context_proto_goTypes,
		DependencyIndexes: file_context_proto_depIdxs,
		MessageInfos:      file_context_proto_msgTypes,
	}.Build()
	File_context_proto = out.File
	file_context_proto_rawDesc = nil
	file_context_proto_goTypes = nil
	file_context_proto_depIdxs = nil
}
