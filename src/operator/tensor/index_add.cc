/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file index_add-inl.cc
 * \brief CPU implementation of index_add operator
*/
#include <vector>
#include "./index_add-inl.h"

namespace mxnet {
namespace op {

template<typename xpu, typename DType, typename VType, int NDim>
void IndexAddForwardImpl(mshadow::Stream<xpu> *s,
                         const int ind_num, DType* out,
                        const VType* val,
                        const mshadow::Shape<NDim>& a_tail_shape,
                        const mshadow::Shape<NDim>& a_pre_stride,
                        const mshadow::Shape<NDim>& val_stride,
                        const mshadow::Shape<NDim>& val_shape,
                        const size_t a_tail_size,
                        const int ind_ndim, const int* ind_vec,
                        const int req) {
  using namespace mxnet_op;
  using namespace mshadow;
  Kernel<IndexAddForwardKernel<DType, VType, NDim>, xpu>::Launch(
                                             s, ind_num, out, val,
                                             a_tail_shape, a_pre_stride,
                                             val_stride, val_shape,
                                             a_tail_size, ind_num,
                                             ind_ndim, ind_vec, req);
}

DMLC_REGISTER_PARAMETER(IndexAddParam);

NNVM_REGISTER_OP(_npx_index_add)
.describe(R"code(This operators implements the "+=" mimic function.
)code" ADD_FILELINE)
.set_attr_parser(ParamParser<IndexAddParam>)
.set_num_inputs(2)
.set_num_outputs(1)
.set_attr<nnvm::FListInputNames>("FListInputNames",
  [](const NodeAttrs& attrs) {
    return std::vector<std::string>{"a", "val"};
  })
.set_attr<mxnet::FInferShape>("FInferShape", IndexAddOpShape)
.set_attr<nnvm::FInferType>("FInferType", IndexAddOpType)
.set_attr<FCompute>("FCompute<cpu>", IndexAddOpForward<cpu>)
.add_argument("a", "NDArray-or-Symbol", "Input ndarray")
.add_argument("val", "NDArray-or-Symbol", "Input ndarray")
.add_arguments(IndexAddParam::__FIELDS__());

}  // namespace op
}  // namespace mxnet

