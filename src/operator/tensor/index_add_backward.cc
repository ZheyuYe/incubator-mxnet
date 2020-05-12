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

template<typename xpu, typename DType, typename OType, int NDim>
void IndexAddOpBackwardACalc(mshadow::Stream<xpu> *s,
                             DType* grad_a, const OType* ograd,
                             const mshadow::Shape<NDim>& stride,
                             const size_t tail_size, const int ind_num,
                             const int ind_ndim, const int* ind_vec,
                             const int req) {
  using namespace mxnet_op;
  using namespace mshadow;
  Kernel<IndexAddBackwardAKernel<DType, OType, NDim>, xpu>::Launch(
    s, ind_num, grad_a, ograd, stride, tail_size, ind_num, ind_ndim, ind_vec, req);
}

template<typename DType, typename OType, int NDim>
struct IndexAddBackwardValCPUKernel {
  MSHADOW_XINLINE static void Map(size_t i, DType* grad_val,
                                  const OType* ograd,
                                  const mshadow::Shape<NDim> ograd_tail_shape,
                                  const mshadow::Shape<NDim> ograd_pre_stride,
                                  const mshadow::Shape<NDim> val_stride,
                                  const mshadow::Shape<NDim> val_shape,
                                  const size_t ograd_tail_size, const int ind_num,
                                  const int ind_ndim, const int* ind_vec) {
    size_t id = 0;
    for (int dim = 0; dim < ind_ndim; ++dim) {
      id += ograd_pre_stride[dim] * ind_vec[dim * ind_num + i];
    }
    id *= ograd_tail_size;
    #pragma omp parallel for
    for (int _i = 0; _i < ograd_tail_size; ++_i) {
      mshadow::Shape<NDim> ograd_tail_id = mxnet_op::unravel(_i, ograd_tail_shape);
      mshadow::Shape<NDim> val_id;
      for (int _j = 0; _j < NDim; ++_j) {
        val_id[_j] = (val_shape[_j] == 1) ? 0 : ograd_tail_id[_j];
      }
      val_id[ind_ndim - 1] = (val_shape[ind_ndim - 1] == 1) ? 0 : i;
      size_t val_dest = mxnet_op::dot(val_id, val_stride);
      #pragma omp critical
      {
        grad_val[val_dest] += static_cast<DType>(ograd[id + _i]);
      }
    }
  }
};

template<typename xpu, typename DType, typename OType, int NDim>
void IndexAddOpBackwardValCalc(mshadow::Stream<xpu> *s,
                               DType* grad_val, const OType* ograd,
                               const mshadow::Shape<NDim>& ograd_tail_shape,
                               const mshadow::Shape<NDim>& ograd_pre_stride,
                               const mshadow::Shape<NDim>& val_stride,
                               const mshadow::Shape<NDim>& val_shape,
                               const size_t tail_size, const int ind_num,
                               const int ind_ndim, const int* ind_vec) {
  using namespace mxnet_op;
  using namespace mshadow;
  Kernel<IndexAddBackwardValCPUKernel<DType, OType, NDim>, xpu>::Launch(
    s, ind_num, grad_val, ograd, ograd_tail_shape, ograd_pre_stride,
    val_stride, val_shape, tail_size, ind_num, ind_ndim, ind_vec);
}

NNVM_REGISTER_OP(_backward_index_add)
.set_num_inputs(3)
.set_num_outputs(2)
.set_attr_parser(ParamParser<IndexModifyParam>)
.set_attr<nnvm::TIsBackward>("TIsBackward", true)
.set_attr<FCompute>("FCompute<cpu>", IndexAddOpBackward<cpu>);

}  // namespace op
}  // namespace mxnet

