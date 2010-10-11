/*
 *  Copyright 2008-2009 NVIDIA Corporation
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

#include <cusp/detail/config.h>

#include <cusp/format.h>
#include <cusp/exception.h>

#include <thrust/copy.h>
#include <thrust/host_vector.h>
#include <thrust/device_allocator.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/device_vector.h>
#include <thrust/detail/vector_base.h>

#include <vector>

namespace cusp
{
  typedef thrust::device_space_tag device_memory;
  typedef thrust::host_space_tag   host_memory;
  
   template<typename T, typename MemorySpace>
   struct choose_memory_allocator
      : thrust::detail::eval_if<
          thrust::detail::is_convertible<MemorySpace, host_memory>::value,
  
          thrust::detail::identity_< std::allocator<T> >,
  
          // XXX add backend-specific allocators here?
  
          thrust::detail::eval_if<
            thrust::detail::is_convertible<MemorySpace, device_memory>::value,
  
            thrust::detail::identity_< thrust::device_malloc_allocator<T> >,
  
            thrust::detail::identity_< MemorySpace >
          >
        >
  {};
  
  
  template <typename T, typename MemorySpace>
  class array1d : public thrust::detail::vector_base<T, typename choose_memory_allocator<T, MemorySpace>::type>
  {
      private:
          typedef typename choose_memory_allocator<T, MemorySpace>::type Alloc;
  
      public:
          typedef MemorySpace memory_space;
          typedef cusp::array1d_format format;
  
          typedef typename thrust::detail::vector_base<T,Alloc> Parent;
          typedef typename Parent::size_type  size_type;
          typedef typename Parent::value_type value_type;
  
          array1d(void) : Parent() {}
          
          explicit array1d(size_type n)
              : Parent()
          {
              if(n > 0)
              {
#if (THRUST_VERSION < 100300)
                  Parent::mBegin = Parent::mAllocator.allocate(n);
                  Parent::mSize  = Parent::mCapacity = n;
#else                    
                  Parent::m_storage.allocate(n);
                  Parent::m_size = n;
#endif
              }
          }
  
          array1d(size_type n, const value_type &value) 
              : Parent(n, value) {}
  
          array1d(const array1d &v)
              : Parent(v) {}
  
          template<typename OtherT, typename OtherAlloc>
              array1d(const array1d<OtherT,OtherAlloc> &v)
              : Parent(v) {}
  
          template<typename OtherT, typename OtherAlloc>
              array1d &operator=(const array1d<OtherT,OtherAlloc> &v)
              { Parent::operator=(v); return *this; }
  
          template<typename OtherT, typename OtherAlloc>
          array1d(const thrust::detail::vector_base<OtherT,OtherAlloc> &v)
              : Parent(v) {}
          
          template<typename OtherT, typename OtherAlloc>
              array1d &operator=(const thrust::detail::vector_base<OtherT,OtherAlloc> &v)
              { Parent::operator=(v); return *this;}
  
          template<typename OtherT, typename OtherAlloc>
              array1d(const std::vector<OtherT,OtherAlloc> &v)
              : Parent(v) {}
  
          template<typename OtherT, typename OtherAlloc>
              array1d &operator=(const std::vector<OtherT,OtherAlloc> &v)
              { Parent::operator=(v); return *this;}
  
          template<typename InputIterator>
              array1d(InputIterator first, InputIterator last)
              : Parent(first, last) {}
  };
  
  template <typename RandomAccessIterator>
  class array1d_view
  {
    public:
      // what about const_iterator and const_reference?
      typedef RandomAccessIterator                                            iterator;
      typedef cusp::array1d_format                                            format;
      typedef typename thrust::iterator_reference<RandomAccessIterator>::type reference;
      typedef typename thrust::iterator_value<RandomAccessIterator>::type     value_type;
      typedef typename thrust::iterator_space<RandomAccessIterator>::type     memory_space;
  
      // is this right?
      typedef size_t size_type;
  
      array1d_view(RandomAccessIterator first, RandomAccessIterator last)
        : m_begin(first), m_size(last - first), m_capacity(last - first) {}
      
      template <typename Array>
      array1d_view &operator=(const Array &a)
      {
        // TODO check size() == a.size()
        thrust::copy(a.begin(), a.end(), begin());
        return *this;
      }
 
      // XXX is this correct?  do we need const_reference?
      reference operator[](size_type n) const
      {
        return m_begin[n];
      }
  
      iterator begin(void)
      {
        return m_begin;
      }
  
      iterator end(void)
      {
        return m_begin + m_size;
      }
  
      size_type size(void) const
      {
        return m_size;
      }
  
      size_type capacity(void) const
      {
        return m_capacity;
      }
  
      // TODO is there any value in supporting the two-argument form?
      //      i.e.  void resize(size_type new_size, value_type x = value_type())
      void resize(size_type new_size)
      {
        if (new_size < m_capacity)
          m_size = new_size;
        else
          // XXX is not_implemented_exception the right choice?
          throw cusp::not_implemented_exception("array1d_view cannot resize() larger than capacity()");
      }
  
    protected:
      iterator  m_begin;
      size_type m_size;
      size_type m_capacity;
  };
  
  template <typename Iterator>
  array1d_view<Iterator> make_array1d_view(Iterator first, Iterator last)
  {
    return array1d_view<Iterator>(first, last);
  }
  
} // end namespace cusp

#include <cusp/detail/array1d.inl>

