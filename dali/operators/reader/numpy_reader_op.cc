// Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "dali/kernels/common/transpose.h"
#include "dali/core/static_switch.h"
#include "dali/operators/reader/numpy_reader_op.h"

namespace dali {

#define NUMPY_ALLOWED_TYPES \
  (bool, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, float, float16, \
  double)

void NumpyReader::TransposeHelper(Tensor<CPUBackend>& output, const Tensor<CPUBackend>& input) {
  auto& in_shape = input.shape();
  auto& type = input.type();
  int n_dims = in_shape.size();
  std::vector<int> perm(n_dims);
  std::vector<int64_t> out_shape(n_dims);
  for (int i = 0; i < n_dims; ++i) {
    perm[i] = n_dims - i - 1;
    out_shape[i] = in_shape[perm[i]];
  }
  output.Resize(out_shape, type);
  auto input_type = type.id();
  TensorShape<> in_ts(in_shape.begin(), in_shape.end());
  TensorShape<> out_ts(out_shape.begin(), out_shape.end());
  TYPE_SWITCH(input_type, type2id, InputType, NUMPY_ALLOWED_TYPES, (
    kernels::Transpose(
      TensorView<StorageCPU, InputType>{output.mutable_data<InputType>(), out_ts},
      TensorView<StorageCPU, const InputType>{input.data<InputType>(), in_ts},
      make_cspan(perm));), DALI_FAIL("Input type not supported."));
}

DALI_REGISTER_OPERATOR(NumpyReader, NumpyReader, CPU);

DALI_SCHEMA(NumpyReader)
  .DocStr("Read Numpy arrays from a directory")
  .NumInput(0)
  .NumOutput(1)  // (Arrays)
  .AddArg("file_root",
      R"code(Path to a directory containing data files.
`NumpyReader` supports flat directory structure. `file_root` directory should contain
directories with numpy files in them.)code",
      DALI_STRING)
  .AddOptionalArg("file_filter",
      R"code(If specified, the string will be interpreted as glob string to filter
the list of files in the sub-directories of `file_root`.)code", "*.npy")
  .AddOptionalArg("shuffle_after_epoch",
      R"code(If true, reader shuffles whole dataset after each epoch. It is exclusive with
`stick_to_shard` and `random_shuffle`.)code",
      false)
  .AddParent("LoaderBase");

}  // namespace dali
