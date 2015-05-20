unsigned char cell_shading_frag[] = {
  0x70, 0x72, 0x65, 0x63, 0x69, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x6d, 0x65,
  0x64, 0x69, 0x75, 0x6d, 0x70, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3b,
  0x0a, 0x0a, 0x75, 0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x76, 0x65,
  0x63, 0x33, 0x20, 0x75, 0x5f, 0x6c, 0x69, 0x67, 0x68, 0x74, 0x5f, 0x70,
  0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3b, 0x0a, 0x75, 0x6e, 0x69,
  0x66, 0x6f, 0x72, 0x6d, 0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x75, 0x5f,
  0x61, 0x6d, 0x62, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x6d, 0x61, 0x74, 0x65,
  0x72, 0x69, 0x61, 0x6c, 0x3b, 0x0a, 0x75, 0x6e, 0x69, 0x66, 0x6f, 0x72,
  0x6d, 0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x75, 0x5f, 0x73, 0x70, 0x65,
  0x63, 0x75, 0x6c, 0x61, 0x72, 0x5f, 0x6d, 0x61, 0x74, 0x65, 0x72, 0x69,
  0x61, 0x6c, 0x3b, 0x0a, 0x75, 0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20,
  0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x72, 0x32, 0x44, 0x20, 0x75, 0x5f,
  0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x3b, 0x0a, 0x75, 0x6e, 0x69, 0x66,
  0x6f, 0x72, 0x6d, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x75, 0x5f,
  0x73, 0x68, 0x69, 0x6e, 0x69, 0x6e, 0x65, 0x73, 0x73, 0x3b, 0x0a, 0x0a,
  0x76, 0x61, 0x72, 0x79, 0x69, 0x6e, 0x67, 0x20, 0x76, 0x65, 0x63, 0x33,
  0x20, 0x76, 0x5f, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3b, 0x0a, 0x76, 0x61,
  0x72, 0x79, 0x69, 0x6e, 0x67, 0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x76,
  0x5f, 0x65, 0x79, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x5f, 0x6e, 0x6f,
  0x72, 0x6d, 0x61, 0x6c, 0x3b, 0x0a, 0x76, 0x61, 0x72, 0x79, 0x69, 0x6e,
  0x67, 0x20, 0x76, 0x65, 0x63, 0x32, 0x20, 0x76, 0x5f, 0x74, 0x65, 0x78,
  0x63, 0x6f, 0x6f, 0x72, 0x64, 0x3b, 0x0a, 0x0a, 0x76, 0x6f, 0x69, 0x64,
  0x20, 0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x20, 0x7b, 0x0a, 0x09, 0x76,
  0x65, 0x63, 0x33, 0x20, 0x6e, 0x20, 0x3d, 0x20, 0x6e, 0x6f, 0x72, 0x6d,
  0x61, 0x6c, 0x69, 0x7a, 0x65, 0x28, 0x76, 0x5f, 0x65, 0x79, 0x65, 0x73,
  0x70, 0x61, 0x63, 0x65, 0x5f, 0x6e, 0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x29,
  0x3b, 0x0a, 0x09, 0x76, 0x65, 0x63, 0x33, 0x20, 0x6c, 0x20, 0x3d, 0x20,
  0x6e, 0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x69, 0x7a, 0x65, 0x28, 0x75, 0x5f,
  0x6c, 0x69, 0x67, 0x68, 0x74, 0x5f, 0x70, 0x6f, 0x73, 0x69, 0x74, 0x69,
  0x6f, 0x6e, 0x29, 0x3b, 0x0a, 0x09, 0x76, 0x65, 0x63, 0x33, 0x20, 0x65,
  0x20, 0x3d, 0x20, 0x76, 0x65, 0x63, 0x33, 0x28, 0x30, 0x2c, 0x20, 0x30,
  0x2c, 0x20, 0x31, 0x29, 0x3b, 0x0a, 0x09, 0x76, 0x65, 0x63, 0x33, 0x20,
  0x68, 0x20, 0x3d, 0x20, 0x6e, 0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x69, 0x7a,
  0x65, 0x28, 0x6c, 0x20, 0x2b, 0x20, 0x65, 0x29, 0x3b, 0x0a, 0x0a, 0x09,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x64, 0x66, 0x20, 0x3d, 0x20, 0x6d,
  0x61, 0x78, 0x28, 0x30, 0x2e, 0x30, 0x2c, 0x20, 0x64, 0x6f, 0x74, 0x28,
  0x6e, 0x2c, 0x20, 0x6c, 0x29, 0x29, 0x3b, 0x0a, 0x09, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x20, 0x73, 0x66, 0x20, 0x3d, 0x20, 0x6d, 0x61, 0x78, 0x28,
  0x30, 0x2e, 0x30, 0x2c, 0x20, 0x64, 0x6f, 0x74, 0x28, 0x6e, 0x2c, 0x20,
  0x68, 0x29, 0x29, 0x3b, 0x0a, 0x09, 0x73, 0x66, 0x20, 0x3d, 0x20, 0x70,
  0x6f, 0x77, 0x28, 0x73, 0x66, 0x2c, 0x20, 0x75, 0x5f, 0x73, 0x68, 0x69,
  0x6e, 0x69, 0x6e, 0x65, 0x73, 0x73, 0x29, 0x3b, 0x0a, 0x09, 0x69, 0x66,
  0x20, 0x28, 0x64, 0x66, 0x20, 0x3c, 0x20, 0x30, 0x2e, 0x31, 0x29, 0x20,
  0x64, 0x66, 0x20, 0x3d, 0x20, 0x30, 0x2e, 0x31, 0x3b, 0x0a, 0x09, 0x65,
  0x6c, 0x73, 0x65, 0x20, 0x69, 0x66, 0x20, 0x28, 0x64, 0x66, 0x20, 0x3c,
  0x20, 0x30, 0x2e, 0x33, 0x29, 0x20, 0x64, 0x66, 0x20, 0x3d, 0x20, 0x30,
  0x2e, 0x33, 0x3b, 0x0a, 0x09, 0x65, 0x6c, 0x73, 0x65, 0x20, 0x69, 0x66,
  0x20, 0x28, 0x64, 0x66, 0x20, 0x3c, 0x20, 0x30, 0x2e, 0x36, 0x29, 0x20,
  0x64, 0x66, 0x20, 0x3d, 0x20, 0x30, 0x2e, 0x36, 0x3b, 0x0a, 0x09, 0x65,
  0x6c, 0x73, 0x65, 0x20, 0x64, 0x66, 0x20, 0x3d, 0x20, 0x31, 0x2e, 0x30,
  0x3b, 0x0a, 0x09, 0x0a, 0x09, 0x73, 0x66, 0x20, 0x3d, 0x20, 0x73, 0x74,
  0x65, 0x70, 0x28, 0x30, 0x2e, 0x35, 0x2c, 0x20, 0x73, 0x66, 0x29, 0x3b,
  0x0a, 0x0a, 0x09, 0x76, 0x65, 0x63, 0x33, 0x20, 0x63, 0x6f, 0x6c, 0x6f,
  0x72, 0x20, 0x3d, 0x20, 0x75, 0x5f, 0x61, 0x6d, 0x62, 0x69, 0x65, 0x6e,
  0x74, 0x5f, 0x6d, 0x61, 0x74, 0x65, 0x72, 0x69, 0x61, 0x6c, 0x20, 0x2b,
  0x20, 0x64, 0x66, 0x20, 0x2a, 0x20, 0x76, 0x5f, 0x63, 0x6f, 0x6c, 0x6f,
  0x72, 0x20, 0x2b, 0x20, 0x73, 0x66, 0x20, 0x2a, 0x20, 0x75, 0x5f, 0x73,
  0x70, 0x65, 0x63, 0x75, 0x6c, 0x61, 0x72, 0x5f, 0x6d, 0x61, 0x74, 0x65,
  0x72, 0x69, 0x61, 0x6c, 0x3b, 0x0a, 0x09, 0x67, 0x6c, 0x5f, 0x46, 0x72,
  0x61, 0x67, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x20, 0x3d, 0x20, 0x76, 0x65,
  0x63, 0x34, 0x28, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x2c, 0x20, 0x31, 0x2e,
  0x30, 0x29, 0x20, 0x2a, 0x20, 0x30, 0x2e, 0x38, 0x20, 0x2b, 0x20, 0x74,
  0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x32, 0x44, 0x28, 0x75, 0x5f, 0x73,
  0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2c, 0x20, 0x76, 0x5f, 0x74, 0x65, 0x78,
  0x63, 0x6f, 0x6f, 0x72, 0x64, 0x29, 0x20, 0x2a, 0x20, 0x30, 0x2e, 0x32,
  0x3b, 0x0a, 0x7d, 0x0a
};
unsigned int cell_shading_frag_len = 784;
