/*! @file median.c
 *
 *  @brief Median filter.
 *
 *  This contains the implementation of the functions for performing a median filter on byte-sized data.
 *
 *  @author 13181680 & 12099115
 *  @date 2018-05-01
 */

 /*!
  *  @addtogroup median_module median module documentation
  *  @{
  */


#include "median.h"
#include "types.h"

/*! @brief Median filters 3 bytes.
 *
 *  @param n1 is the first  of 3 bytes for which the median is sought.
 *  @param n2 is the second of 3 bytes for which the median is sought.
 *  @param n3 is the third  of 3 bytes for which the median is sought.
 *  @note all possible cases are considered
 */
uint8_t Median_Filter3(const uint8_t n1, const uint8_t n2, const uint8_t n3)
{
  if(n1 > n2)
  {
    if(n2 > n3)
    {
      return n2; // n2 is the middle value
    }
    else
    {
      return n3; // n3 is the middle value
    }
  }
  if(n2 > n1)
  {
    if(n1 > n3)
    {
      return n1; // n1 is the middle value
    }
    else
    {
      return n3; // n3 is the middle value
    }
  }
  if(n1 > n3)
  {
    if(n3 > n2)
    {
      return n3; // n3 is the middle value
    }
    else{
      return n2; // n2 is the middle value
    }
  }
  if(n3 > n1)
  {
    if(n1 > n2)
    {
      return n1; // n1 is the middle value
    }
    else
    {
      return n2; // n2 is the middle value
    }
  }
  if(n2 > n3)
  {
    if(n3 > n1)
    {
      return n3; // n3 is the middle value
    }
    else
    {
      return n1; // n1 is the middle value
    }
  }
  if(n3 > n2)
  {
    if(n2 > n1)
    {
      return n2; // n2 is the middle value
    }
    else
    {
      return n1; // n1 is the middle value
    }
  }
}

/*!
 * @}
*/
