#ifndef REALTIME_UTILITIES_CIRCULAR_BUFFER_STAMPED__H
#define REALTIME_UTILITIES_CIRCULAR_BUFFER_STAMPED__H


#include <boost/date_time.hpp>
#include <iostream>
#include <realtime_utilities/circular_buffer.h>

namespace realtime_utilities
{

#include <time.h>

// Thread safe circular buffer 
template <typename T>
class circ_buffer_stamped : public realtime_utilities::circ_buffer<std::tuple< std::string, double, T > >
{
public:
  circ_buffer_stamped(int n) : realtime_utilities::circ_buffer<std::tuple< std::string, double, T > >( n) {}
  void push_back( const std::tuple< std::string, double, T >& error ) override
  {
    realtime_utilities::circ_buffer<std::tuple< std::string, double, T > >::push_back( error );
  }
  void push_back( const T& error ) 
  {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    double      t = now.time_of_day().seconds();
    std::string s = boost::posix_time::to_simple_string( now );

    realtime_utilities::circ_buffer<std::tuple< std::string, double, T > >::push_back( std::make_tuple( s, t, error ) );
  }
};


template <typename T>
class circ_buffer_stamped_named : public realtime_utilities::circ_buffer_stamped< T >
{
public:
  const std::string id;
  circ_buffer_stamped_named(const std::string& i, int n)
    : realtime_utilities::circ_buffer_stamped< T >( n ), id( i )
  {
    //
  }
};

template <typename C, typename T>
class circ_buffer_stamped_checked : public realtime_utilities::circ_buffer_stamped_named< T >
{
public:
  const C check;
  circ_buffer_stamped_checked(const std::string& i, const C& c, int n)
    : realtime_utilities::circ_buffer_stamped_named< T >( i, n ), check( c )
  {
    //
  }
  bool push_back( const T& c, const T& val )
  {
    if( c == check )
      return false;

    return push_back(val);
  }

};









}

#endif
