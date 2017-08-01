/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


#ifndef INCLUDED_OFDM_ALAMOUTI_ALAMOUTI_TX_H
#define INCLUDED_OFDM_ALAMOUTI_ALAMOUTI_TX_H

#include <ofdm_alamouti/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace ofdm_alamouti {

    /*!
     * \brief <+description of block+>
     * \ingroup ofdm_alamouti
     *
     */
    class OFDM_ALAMOUTI_API Alamouti_TX : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<Alamouti_TX> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of ofdm_alamouti::Alamouti_TX.
       *
       * To avoid accidental use of raw pointers, ofdm_alamouti::Alamouti_TX's
       * constructor is in a private implementation
       * class. ofdm_alamouti::Alamouti_TX::make is the public interface for
       * creating new instances.
       */
      static sptr make(size_t N_data_subc,size_t N_preamble_subc, size_t channel_len);
    };

  } // namespace ofdm_alamouti
} // namespace gr

#endif /* INCLUDED_OFDM_ALAMOUTI_ALAMOUTI_TX_H */

