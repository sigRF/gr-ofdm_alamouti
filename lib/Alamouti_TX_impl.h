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

#ifndef INCLUDED_OFDM_ALAMOUTI_ALAMOUTI_TX_IMPL_H
#define INCLUDED_OFDM_ALAMOUTI_ALAMOUTI_TX_IMPL_H

#include <ofdm_alamouti/Alamouti_TX.h>
#include <gnuradio/fft/fft.h>
#include <math.h>
#define ARMA_DONT_USE_WRAPPER
#include <armadillo>

namespace gr
{
  namespace ofdm_alamouti
  {

    class Alamouti_TX_impl : public Alamouti_TX
    {
    private:
      size_t d_N_data_subc;
      size_t d_N_preamble_subc;
      size_t d_channel_len;

      gr_complex *d_DATA;
      gr_complex *d_PREAMBLE;

      // kapoia data ta thewrw gnwsta ston receiver gia trainning opou tha ftiaktoun gia anixneusi kanaliou
      gr_complex *d_ALAMOUTI1;
      gr_complex *d_ALAMOUTI2;

      gr_complex *d_preamble;
      gr_complex *d_alamouti1;
      gr_complex *d_alamouti2;
      gr_complex *d_preambleCP;
      gr_complex *d_alamouti1CP;
      gr_complex *d_alamouti2CP;

      /* Auxiliary FFT vector */
      fft::fft_complex *d_fft_pr;
      fft::fft_complex *d_fft_da;

      gr_complex *d_tx1_packet;
      gr_complex *d_tx2_packet;

      gr_complex *d_tx1_packet_with_zeros;
      gr_complex *d_tx2_packet_with_zeros;


    public:
      Alamouti_TX_impl (size_t N_data_subc,size_t N_preamble_subc, size_t channel_len);
      ~Alamouti_TX_impl ();

      // Where all the action really happens
      int
      work (int noutput_items, gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);

      void
      create_data (gr_complex *random_symbols, int random_factor, int step, int n_items);

    };

  } // namespace ofdm_alamouti
} // namespace gr

#endif /* INCLUDED_OFDM_ALAMOUTI_ALAMOUTI_TX_IMPL_H */

