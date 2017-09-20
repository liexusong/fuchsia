#include <stdint.h>

#if 0
../cube.vert
Warning, version 400 is not yet complete; most version-specific features are present, but some are missing.


Linked vertex stage:


// Module Version 10000
// Generated by (magic number): 80001
// Id's are bound by 55

                              Capability Shader
               1:             ExtInstImport  "GLSL.std.450"
                              MemoryModel Logical GLSL450
                              EntryPoint Vertex 4  "main" 9 21 28
                              Source GLSL 400
                              SourceExtension  "GL_ARB_separate_shader_objects"
                              SourceExtension  "GL_ARB_shading_language_420pack"
                              Name 4  "main"
                              Name 9  "texcoord"
                              Name 15  "buf"
                              MemberName 15(buf) 0  "MVP"
                              MemberName 15(buf) 1  "position"
                              MemberName 15(buf) 2  "attr"
                              Name 17  "ubuf"
                              Name 21  "gl_VertexIndex"
                              Name 26  "gl_PerVertex"
                              MemberName 26(gl_PerVertex) 0  "gl_Position"
                              Name 28  ""
                              Decorate 9(texcoord) Location 0
                              Decorate 13 ArrayStride 16
                              Decorate 14 ArrayStride 16
                              MemberDecorate 15(buf) 0 ColMajor
                              MemberDecorate 15(buf) 0 Offset 0
                              MemberDecorate 15(buf) 0 MatrixStride 16
                              MemberDecorate 15(buf) 1 Offset 64
                              MemberDecorate 15(buf) 2 Offset 640
                              Decorate 15(buf) Block
                              Decorate 17(ubuf) DescriptorSet 0
                              Decorate 17(ubuf) Binding 0
                              Decorate 21(gl_VertexIndex) BuiltIn VertexIndex
                              MemberDecorate 26(gl_PerVertex) 0 BuiltIn Position
                              Decorate 26(gl_PerVertex) Block
               2:             TypeVoid
               3:             TypeFunction 2
               6:             TypeFloat 32
               7:             TypeVector 6(float) 4
               8:             TypePointer Output 7(fvec4)
     9(texcoord):      8(ptr) Variable Output
              10:             TypeMatrix 7(fvec4) 4
              11:             TypeInt 32 0
              12:     11(int) Constant 36
              13:             TypeArray 7(fvec4) 12
              14:             TypeArray 7(fvec4) 12
         15(buf):             TypeStruct 10 13 14
              16:             TypePointer Uniform 15(buf)
        17(ubuf):     16(ptr) Variable Uniform
              18:             TypeInt 32 1
              19:     18(int) Constant 2
              20:             TypePointer Input 18(int)
21(gl_VertexIndex):     20(ptr) Variable Input
              23:             TypePointer Uniform 7(fvec4)
26(gl_PerVertex):             TypeStruct 7(fvec4)
              27:             TypePointer Output 26(gl_PerVertex)
              28:     27(ptr) Variable Output
              29:     18(int) Constant 0
              30:             TypePointer Uniform 10
              33:     18(int) Constant 1
              39:     11(int) Constant 1
              40:             TypePointer Output 6(float)
              45:     11(int) Constant 2
              48:     11(int) Constant 3
              52:    6(float) Constant 1073741824
         4(main):           2 Function None 3
               5:             Label
              22:     18(int) Load 21(gl_VertexIndex)
              24:     23(ptr) AccessChain 17(ubuf) 19 22
              25:    7(fvec4) Load 24
                              Store 9(texcoord) 25
              31:     30(ptr) AccessChain 17(ubuf) 29
              32:          10 Load 31
              34:     18(int) Load 21(gl_VertexIndex)
              35:     23(ptr) AccessChain 17(ubuf) 33 34
              36:    7(fvec4) Load 35
              37:    7(fvec4) MatrixTimesVector 32 36
              38:      8(ptr) AccessChain 28 29
                              Store 38 37
              41:     40(ptr) AccessChain 28 29 39
              42:    6(float) Load 41
              43:    6(float) FNegate 42
              44:     40(ptr) AccessChain 28 29 39
                              Store 44 43
              46:     40(ptr) AccessChain 28 29 45
              47:    6(float) Load 46
              49:     40(ptr) AccessChain 28 29 48
              50:    6(float) Load 49
              51:    6(float) FAdd 47 50
              53:    6(float) FDiv 51 52
              54:     40(ptr) AccessChain 28 29 45
                              Store 54 53
                              Return
                              FunctionEnd
#endif

static const uint32_t cube_vert[396] = {
    0x07230203, 0x00010000, 0x00080001, 0x00000037, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0008000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00000015, 0x0000001c,
    0x00030003, 0x00000002, 0x00000190, 0x00090004, 0x415f4c47, 0x735f4252, 0x72617065, 0x5f657461,
    0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00090004, 0x415f4c47, 0x735f4252, 0x69646168,
    0x6c5f676e, 0x75676e61, 0x5f656761, 0x70303234, 0x006b6361, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00050005, 0x00000009, 0x63786574, 0x64726f6f, 0x00000000, 0x00030005, 0x0000000f,
    0x00667562, 0x00040006, 0x0000000f, 0x00000000, 0x0050564d, 0x00060006, 0x0000000f, 0x00000001,
    0x69736f70, 0x6e6f6974, 0x00000000, 0x00050006, 0x0000000f, 0x00000002, 0x72747461, 0x00000000,
    0x00040005, 0x00000011, 0x66756275, 0x00000000, 0x00060005, 0x00000015, 0x565f6c67, 0x65747265,
    0x646e4978, 0x00007865, 0x00060005, 0x0000001a, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
    0x00060006, 0x0000001a, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x0000001c,
    0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x00000006,
    0x00000010, 0x00040047, 0x0000000e, 0x00000006, 0x00000010, 0x00040048, 0x0000000f, 0x00000000,
    0x00000005, 0x00050048, 0x0000000f, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000000f,
    0x00000000, 0x00000007, 0x00000010, 0x00050048, 0x0000000f, 0x00000001, 0x00000023, 0x00000040,
    0x00050048, 0x0000000f, 0x00000002, 0x00000023, 0x00000280, 0x00030047, 0x0000000f, 0x00000002,
    0x00040047, 0x00000011, 0x00000022, 0x00000000, 0x00040047, 0x00000011, 0x00000021, 0x00000000,
    0x00040047, 0x00000015, 0x0000000b, 0x0000002a, 0x00050048, 0x0000001a, 0x00000000, 0x0000000b,
    0x00000000, 0x00030047, 0x0000001a, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
    0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003,
    0x00040018, 0x0000000a, 0x00000007, 0x00000004, 0x00040015, 0x0000000b, 0x00000020, 0x00000000,
    0x0004002b, 0x0000000b, 0x0000000c, 0x00000024, 0x0004001c, 0x0000000d, 0x00000007, 0x0000000c,
    0x0004001c, 0x0000000e, 0x00000007, 0x0000000c, 0x0005001e, 0x0000000f, 0x0000000a, 0x0000000d,
    0x0000000e, 0x00040020, 0x00000010, 0x00000002, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011,
    0x00000002, 0x00040015, 0x00000012, 0x00000020, 0x00000001, 0x0004002b, 0x00000012, 0x00000013,
    0x00000002, 0x00040020, 0x00000014, 0x00000001, 0x00000012, 0x0004003b, 0x00000014, 0x00000015,
    0x00000001, 0x00040020, 0x00000017, 0x00000002, 0x00000007, 0x0003001e, 0x0000001a, 0x00000007,
    0x00040020, 0x0000001b, 0x00000003, 0x0000001a, 0x0004003b, 0x0000001b, 0x0000001c, 0x00000003,
    0x0004002b, 0x00000012, 0x0000001d, 0x00000000, 0x00040020, 0x0000001e, 0x00000002, 0x0000000a,
    0x0004002b, 0x00000012, 0x00000021, 0x00000001, 0x0004002b, 0x0000000b, 0x00000027, 0x00000001,
    0x00040020, 0x00000028, 0x00000003, 0x00000006, 0x0004002b, 0x0000000b, 0x0000002d, 0x00000002,
    0x0004002b, 0x0000000b, 0x00000030, 0x00000003, 0x0004002b, 0x00000006, 0x00000034, 0x40000000,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
    0x00000012, 0x00000016, 0x00000015, 0x00060041, 0x00000017, 0x00000018, 0x00000011, 0x00000013,
    0x00000016, 0x0004003d, 0x00000007, 0x00000019, 0x00000018, 0x0003003e, 0x00000009, 0x00000019,
    0x00050041, 0x0000001e, 0x0000001f, 0x00000011, 0x0000001d, 0x0004003d, 0x0000000a, 0x00000020,
    0x0000001f, 0x0004003d, 0x00000012, 0x00000022, 0x00000015, 0x00060041, 0x00000017, 0x00000023,
    0x00000011, 0x00000021, 0x00000022, 0x0004003d, 0x00000007, 0x00000024, 0x00000023, 0x00050091,
    0x00000007, 0x00000025, 0x00000020, 0x00000024, 0x00050041, 0x00000008, 0x00000026, 0x0000001c,
    0x0000001d, 0x0003003e, 0x00000026, 0x00000025, 0x00060041, 0x00000028, 0x00000029, 0x0000001c,
    0x0000001d, 0x00000027, 0x0004003d, 0x00000006, 0x0000002a, 0x00000029, 0x0004007f, 0x00000006,
    0x0000002b, 0x0000002a, 0x00060041, 0x00000028, 0x0000002c, 0x0000001c, 0x0000001d, 0x00000027,
    0x0003003e, 0x0000002c, 0x0000002b, 0x00060041, 0x00000028, 0x0000002e, 0x0000001c, 0x0000001d,
    0x0000002d, 0x0004003d, 0x00000006, 0x0000002f, 0x0000002e, 0x00060041, 0x00000028, 0x00000031,
    0x0000001c, 0x0000001d, 0x00000030, 0x0004003d, 0x00000006, 0x00000032, 0x00000031, 0x00050081,
    0x00000006, 0x00000033, 0x0000002f, 0x00000032, 0x00050088, 0x00000006, 0x00000035, 0x00000033,
    0x00000034, 0x00060041, 0x00000028, 0x00000036, 0x0000001c, 0x0000001d, 0x0000002d, 0x0003003e,
    0x00000036, 0x00000035, 0x000100fd, 0x00010038,
};