/*
 *  Copyright 2008-2014 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/permutation_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/functional.h>

namespace cusp
{

// Integer hash functions
template <typename IndexType, typename T>
struct random_integer_functor : public thrust::unary_function<IndexType,T>
{
    size_t seed;

    random_integer_functor(const size_t seed)
        : seed(seed) {}

    // source: http://www.concentric.net/~ttwang/tech/inthash.htm
    __host__ __device__
    T hash(const IndexType i, thrust::detail::false_type) const
    {
        unsigned int h = (unsigned int) i ^ (unsigned int) seed;
        h = ~h + (h << 15);
        h =  h ^ (h >> 12);
        h =  h + (h <<  2);
        h =  h ^ (h >>  4);
        h =  h + (h <<  3) + (h << 11);
        h =  h ^ (h >> 16);
        return T(h);
    }

    __host__ __device__
    T hash(const IndexType i, thrust::detail::true_type) const
    {
        unsigned long long h = (unsigned long long) i ^ (unsigned long long) seed;
        h = ~h + (h << 21);
        h =  h ^ (h >> 24);
        h = (h + (h <<  3)) + (h << 8);
        h =  h ^ (h >> 14);
        h = (h + (h <<  2)) + (h << 4);
        h =  h ^ (h >> 28);
        h =  h + (h << 31);
        return T(h);
    }

    __host__ __device__
    T operator()(const IndexType i) const
    {
        return hash(i, typename thrust::detail::integral_constant<bool, sizeof(IndexType) == 8 || sizeof(T) == 8>::type());
    }
};

template<typename T>
class random_iterator
{
public:

    typedef std::ptrdiff_t                                                               IndexType;
    typedef random_integer_functor<IndexType,T>                                          IndexFunctor;
    typedef typename thrust::counting_iterator<IndexType>                                RandomCountingIterator;
    typedef typename thrust::transform_iterator<IndexFunctor, RandomCountingIterator, T> RandomTransformIterator;

    // type of the random_range iterator
    typedef RandomTransformIterator                                                      iterator;

    IndexFunctor index_func;

    random_iterator(size_t seed)
        : index_func(seed) {}

    iterator begin(void) const
    {
        return RandomTransformIterator(RandomTransformIterator(RandomCountingIterator(0), index_func), index_func);
    }
};

} // end namespace cusp

