/* -*- c++ -*- */

#define OFDM_ALAMOUTI_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "ofdm_alamouti_swig_doc.i"

%{
#include "ofdm_alamouti/Alamouti_TX.h"
%}


%include "ofdm_alamouti/Alamouti_TX.h"
GR_SWIG_BLOCK_MAGIC2(ofdm_alamouti, Alamouti_TX);
