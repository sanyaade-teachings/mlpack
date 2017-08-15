/**
 * @file dataset_mapper_impl.hpp
 * @author Ryan Curtin
 * @author Keon Kim
 *
 * An implementation of the DatasetMapper<PolicyType> class.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_CORE_DATA_DATASET_INFO_IMPL_HPP
#define MLPACK_CORE_DATA_DATASET_INFO_IMPL_HPP

// In case it hasn't already been included.
#include "dataset_mapper.hpp"

namespace mlpack {
namespace data {

// Default constructor.
template<typename PolicyType>
inline DatasetMapper<PolicyType>::DatasetMapper(const size_t dimensionality) :
    types(dimensionality, Datatype::numeric)
{
  // Nothing to initialize here.
}

template<typename PolicyType>
inline DatasetMapper<PolicyType>::DatasetMapper(PolicyType& policy,
    const size_t dimensionality) :
    types(dimensionality, Datatype::numeric),
    policy(std::move(policy))
{
  // Nothing to initialize here.
}

// Utility helper function to call MapFirstPass.
template<typename PolicyType, typename T>
void CallMapFirstPass(
    PolicyType& policy,
    const std::string& string,
    const size_t dimension,
    std::vector<Datatype>& types,
    const typename std::enable_if<PolicyType::NeedsFirstPass>::type* = 0)
{
  policy.template MapFirstPass<T>(string, dimension, types);
}

// Utility helper function that doesn't call anything.
template<typename PolicyType, typename T>
void CallMapFirstPass(
    PolicyType& /* policy */,
    const std::string& /* string */,
    const size_t /* dimension */,
    std::vector<Datatype>& /* types */,
    const typename std::enable_if<!PolicyType::NeedsFirstPass>::type* = 0)
{
  // Nothing to do here.
}

template<typename PolicyType>
template<typename T>
void DatasetMapper<PolicyType>::MapFirstPass(const std::string& string,
                                             const size_t dimension)
{
  // Call the correct overload (via SFINAE).
  CallMapFirstPass<PolicyType, T>(policy, string, dimension, types);
}

// When we want to insert value into the map, we use the policy to map the
// string.
template<typename PolicyType>
template<typename T>
inline T DatasetMapper<PolicyType>::MapString(const std::string& string,
                                              const size_t dimension)
{
  return policy.template MapString<MapType, T>(string, dimension, maps, types);
}

// Return the string corresponding to a value in a given dimension.
template<typename PolicyType>
template<typename T>
inline const std::string& DatasetMapper<PolicyType>::UnmapString(
    const T value,
    const size_t dimension,
    const size_t unmappingIndex) const
{
  // If the value is std::numeric_limits<T>::quiet_NaN(), we can't use it as a
  // key---so we will use something else...
  const T usedValue = std::isnan(value) ?
      std::nexttoward(std::numeric_limits<T>::max(), T(0)) :
      value;

  // Throw an exception if the value doesn't exist.
  if (maps.at(dimension).second.count(usedValue) == 0)
  {
    std::ostringstream oss;
    oss << "DatasetMapper<PolicyType>::UnmapString(): value '" << value
        << "' unknown for dimension " << dimension;
    throw std::invalid_argument(oss.str());
  }

  if (unmappingIndex >= maps.at(dimension).second.at(usedValue).size())
  {
    std::ostringstream oss;
    oss << "DatasetMapper<PolicyType>::UnmapString(): value '" << value
        << "' only has " << maps.at(dimension).second.at(usedValue).size()
        << " unmappings, but unmappingIndex is " << unmappingIndex << "!";
    throw std::invalid_argument(oss.str());
  }

  return maps.at(dimension).second.at(usedValue)[unmappingIndex];
}

template<typename PolicyType>
template<typename T>
inline size_t DatasetMapper<PolicyType>::NumUnmappings(
    const T value,
    const size_t dimension) const
{
  // If the value is std::numeric_limits<T>::quiet_NaN(), we can't use it as a
  // key---so we will use something else...
  if (std::isnan(value))
  {
    const T newValue = std::nexttoward(std::numeric_limits<T>::max(), T(0));
    return maps.at(dimension).second.at(newValue).size();
  }

  return maps.at(dimension).second.at(value).size();
}

// Return the value corresponding to a string in a given dimension.
template<typename PolicyType>
inline typename PolicyType::MappedType DatasetMapper<PolicyType>::UnmapValue(
    const std::string& string,
    const size_t dimension)
{
  // Throw an exception if the value doesn't exist.
  if (maps[dimension].first.count(string) == 0)
  {
    std::ostringstream oss;
    oss << "DatasetMapper<PolicyType>::UnmapValue(): string '" << string
        << "' unknown for dimension " << dimension;
    throw std::invalid_argument(oss.str());
  }

  return maps[dimension].first.at(string);
}

// Get the type of a particular dimension.
template<typename PolicyType>
inline Datatype DatasetMapper<PolicyType>::Type(const size_t dimension) const
{
  if (dimension >= types.size())
  {
    std::ostringstream oss;
    oss << "requested type of dimension " << dimension << ", but dataset only "
        << "has " << types.size() << " dimensions";
    throw std::invalid_argument(oss.str());
  }

  return types[dimension];
}

template<typename PolicyType>
inline Datatype& DatasetMapper<PolicyType>::Type(const size_t dimension)
{
  if (dimension >= types.size())
    types.resize(dimension + 1, Datatype::numeric);

  return types[dimension];
}

template<typename PolicyType>
inline
size_t DatasetMapper<PolicyType>::NumMappings(const size_t dimension) const
{
  return (maps.count(dimension) == 0) ? 0 : maps.at(dimension).first.size();
}

template<typename PolicyType>
inline size_t DatasetMapper<PolicyType>::Dimensionality() const
{
  return types.size();
}

template<typename PolicyType>
inline const PolicyType& DatasetMapper<PolicyType>::Policy() const
{
  return this->policy;
}

template<typename PolicyType>
inline PolicyType& DatasetMapper<PolicyType>::Policy()
{
  return this->policy;
}

template<typename PolicyType>
inline void DatasetMapper<PolicyType>::Policy(PolicyType&& policy)
{
  this->policy = std::forward<PolicyType>(policy);
}

} // namespace data
} // namespace mlpack

#endif
