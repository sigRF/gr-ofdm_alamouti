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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "Alamouti_TX_impl.h"
#include <volk/volk.h>

namespace gr
{
  namespace ofdm_alamouti
  {

    Alamouti_TX::sptr
    Alamouti_TX::make (size_t N_data_subc, size_t N_preamble_subc,
		       size_t channel_len)
    {
      return gnuradio::get_initial_sptr (
	  new Alamouti_TX_impl (N_data_subc, N_preamble_subc, channel_len));
    }

    /*
     * The private constructor
     */
    Alamouti_TX_impl::Alamouti_TX_impl (size_t N_data_subc,
					size_t N_preamble_subc,
					size_t channel_len) :
	    gr::sync_block ("Alamouti_TX", gr::io_signature::make (0, 0, 0),
			    gr::io_signature::make (2, 2, sizeof(gr_complex))),
	    d_N_data_subc (N_data_subc),
	    d_N_preamble_subc (N_preamble_subc),
	    d_channel_len (channel_len)

    {
      d_DATA = (gr_complex*) volk_malloc ((d_N_data_subc) * sizeof(gr_complex),
					  32);
      d_PREAMBLE = (gr_complex*) volk_malloc (
	  (d_N_preamble_subc) * sizeof(gr_complex), 32);
      d_ALAMOUTI1 = (gr_complex*) volk_malloc (
	  (d_N_data_subc) * sizeof(gr_complex), 32);
      d_ALAMOUTI2 = (gr_complex*) volk_malloc (
	  (d_N_data_subc) * sizeof(gr_complex), 32);
      d_preamble = (gr_complex*) volk_malloc (
	  (d_N_preamble_subc) * sizeof(gr_complex), 32);
      d_alamouti1 = (gr_complex*) volk_malloc (
	  (d_N_data_subc) * sizeof(gr_complex), 32);
      d_alamouti2 = (gr_complex*) volk_malloc (
	  (d_N_data_subc) * sizeof(gr_complex), 32);

      d_preambleCP = (gr_complex*) volk_malloc (
	  (d_N_preamble_subc + d_channel_len) * sizeof(gr_complex), 32);
      d_alamouti1CP = (gr_complex*) volk_malloc (
	  (d_N_data_subc + d_channel_len) * sizeof(gr_complex), 32);
      d_alamouti2CP = (gr_complex*) volk_malloc (
	  (d_N_data_subc + d_channel_len) * sizeof(gr_complex), 32);

      memset (d_PREAMBLE, 0, d_N_preamble_subc * sizeof(gr_complex));
      // create random Preambles
      create_data (d_PREAMBLE, 1, 2, d_N_preamble_subc);
      volk_32fc_s32fc_multiply_32fc (d_PREAMBLE, d_PREAMBLE, sqrt (2),
				     d_N_preamble_subc);

      // create random Data
      create_data (d_DATA, 2, 1, d_N_data_subc);

      for (int i = 0; i < d_N_data_subc; i = i + 2) {
	/* first time slot */
	d_ALAMOUTI1[i] = d_DATA[i];
	d_ALAMOUTI2[i] = d_DATA[i + 1];

	/* second time slot */
	d_ALAMOUTI1[i + 1] = -conj (d_DATA[i + 1]);
	d_ALAMOUTI2[i + 1] = conj (d_DATA[i]);

      }

      d_fft_pr = new fft::fft_complex (d_N_preamble_subc, false, 1);
      d_fft_da = new fft::fft_complex (d_N_data_subc, false, 1);

      gr_complex *d_fft_pr_in = d_fft_pr->get_inbuf ();
      gr_complex *d_fft_da_in = d_fft_da->get_inbuf ();

      memcpy (d_fft_pr_in, d_PREAMBLE, d_N_preamble_subc * sizeof(gr_complex));
      d_fft_pr->execute ();
      memcpy (d_preamble, d_fft_pr->get_outbuf (),
	      d_N_preamble_subc * sizeof(gr_complex));

      memcpy (d_fft_da_in, d_ALAMOUTI1, d_N_data_subc * sizeof(gr_complex));
      d_fft_da->execute ();
      memcpy (d_alamouti1, d_fft_da->get_outbuf (),
	      d_N_data_subc * sizeof(gr_complex));

      memcpy (d_fft_da_in, d_ALAMOUTI2, d_N_data_subc * sizeof(gr_complex));
      d_fft_da->execute ();
      memcpy (d_alamouti2, d_fft_da->get_outbuf (),
	      d_N_data_subc * sizeof(gr_complex));

      /* multiply with scalar */
      volk_32fc_s32fc_multiply_32fc (
	  d_preamble, d_preamble, sqrt (d_N_preamble_subc) / d_N_preamble_subc,
	  d_N_preamble_subc);
      volk_32fc_s32fc_multiply_32fc (d_alamouti1, d_alamouti1,
				     sqrt (d_N_data_subc) / d_N_data_subc,
				     d_N_data_subc);
      volk_32fc_s32fc_multiply_32fc (d_alamouti2, d_alamouti2,
				     sqrt (d_N_data_subc) / d_N_data_subc,
				     d_N_data_subc);

      // add CP at alamouti1, alamouti2 and preambles
      for (int i = 0; i < d_channel_len; i++) {
	d_preambleCP[i] = d_preamble[d_N_preamble_subc - d_channel_len + i];
	d_alamouti1CP[i] = d_alamouti1[d_N_data_subc - d_channel_len + i];
	d_alamouti2CP[i] = d_alamouti2[d_N_data_subc - d_channel_len + i];
      }

      memcpy (&d_preambleCP[d_channel_len], d_preamble,
	      d_N_preamble_subc * sizeof(gr_complex));
      memcpy (&d_alamouti1CP[d_channel_len], d_alamouti1,
	      d_N_data_subc * sizeof(gr_complex));
      memcpy (&d_alamouti2CP[d_channel_len], d_alamouti2,
	      d_N_data_subc * sizeof(gr_complex));

      d_tx1_packet = (gr_complex*) volk_malloc (
	  (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex),
	  32);
      d_tx2_packet = (gr_complex*) volk_malloc (
	  (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex),
	  32);

      /* packet 1 for antenna 1*/
      memcpy (d_tx1_packet, d_preambleCP,
	      (d_N_preamble_subc + d_channel_len) * sizeof(gr_complex));
      memcpy (&d_tx1_packet[d_N_preamble_subc + d_channel_len], d_alamouti1CP,
	      (d_N_data_subc + d_channel_len) * sizeof(gr_complex));

      /* packet 2 for antenna 2*/
      memcpy (d_tx2_packet, d_preambleCP,
	      (d_N_preamble_subc + d_channel_len) * sizeof(gr_complex));
      memcpy (&d_tx2_packet[d_N_preamble_subc + d_channel_len], d_alamouti2CP,
	      (d_N_data_subc + d_channel_len) * sizeof(gr_complex));

      /* packets with zeros */
      d_tx1_packet_with_zeros = (gr_complex*) volk_malloc (
	  2 * (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex),
	  32);
      d_tx2_packet_with_zeros = (gr_complex*) volk_malloc (
	  2 * (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex),
	  32);

      memset (
	  d_tx1_packet_with_zeros,
	  0,
	  2 * (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex));
      memset (
	  d_tx2_packet_with_zeros,
	  0,
	  2 * (d_N_data_subc + +d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex));
      memcpy (
	  d_tx1_packet_with_zeros,
	  d_tx1_packet,
	  (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex));
      memcpy (
	  d_tx2_packet_with_zeros,
	  d_tx2_packet,
	  (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex));

      message_port_register_in (pmt::mp ("tx_en"));

    }

    /*
     * Our virtual destructor.
     */
    Alamouti_TX_impl::~Alamouti_TX_impl ()
    {
      volk_free (d_DATA);
      volk_free (d_PREAMBLE);
      volk_free (d_ALAMOUTI1);
      volk_free (d_ALAMOUTI2);
      volk_free (d_preamble);
      volk_free (d_alamouti1);
      volk_free (d_alamouti2);
      volk_free (d_preambleCP);
      volk_free (d_alamouti1CP);
      volk_free (d_alamouti2CP);
      volk_free (d_tx1_packet);
      volk_free (d_tx2_packet);
      volk_free (d_DATA);
      volk_free (d_tx1_packet_with_zeros);
      volk_free (d_tx2_packet_with_zeros);
      delete d_fft_pr;
      delete d_fft_da;
    }

    int
    Alamouti_TX_impl::work (int noutput_items,
			    gr_vector_const_void_star &input_items,
			    gr_vector_void_star &output_items)
    {
      gr_complex *out1 = (gr_complex*) output_items[0];
      gr_complex *out2 = (gr_complex*) output_items[1];

      pmt::pmt_t msg = delete_head_blocking (pmt::mp ("tx_en"));
      memcpy (
	  out1,
	  d_tx1_packet_with_zeros,
	  2 * (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex));
      memcpy (
	  out2,
	  d_tx2_packet_with_zeros,
	  2 * (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len)
	      * sizeof(gr_complex));

      // Tell runtime system how many output items we produced.
      return 2 * (d_N_data_subc + d_N_preamble_subc + 2 * d_channel_len);
    }

    void
    Alamouti_TX_impl::create_data (gr_complex *random_symbols,
				   int random_factor, int step, int n_items)
    {
      for (int i = 0; i < n_items; i = i + step) {
	srand (i * random_factor);
	random_symbols[i].real ((1 - 2 * (rand () % 2)) * 1);  // Real +-1
	random_symbols[i].imag ((1 - 2 * (rand () % 2)) * 1);  // Imag +-1
      }
    }

  } /* namespace ofdm_alamouti */
} /* namespace gr */

