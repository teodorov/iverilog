/*
 * Copyright (c) 1998 Stephen Williams (steve@picturel.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT)
#ident "$Id: propinit.cc,v 1.1 1998/12/20 02:05:41 steve Exp $"
#endif

/*
 * The propinit function runs through the devices that can impose
 * initial values in the netlist and propogates those values. The
 * process works by first scanning the active devices for outputs that
 * they generate.
 */

# include "netlist.h"

/*
 * prop_sequdp_output takes the output from the located sequential UDP
 * device and propogates it to the signals connected to it.
 */
static bool is_sequ_udp(const NetNode*net)
{
      const NetUDP*udp;
      if ((udp = dynamic_cast<const NetUDP*>(net)) == 0)
	    return false;

      return udp->is_sequential();
}

static void prop_sequdp_output(NetUDP*udp)
{
	/* Get from the UDP class the initial output value. */
      verinum::V ival;
      switch (udp->get_initial()) {
	  case '0':
	    ival = verinum::V0;
	    break;
	  case '1':
	    ival = verinum::V1;
	    break;
	  default:
	    ival = verinum::Vx;
	    break;
      }

	/* Take the output value and write it to all the NetNet pins
	   that are connected to the output pin. */

      for (NetObj::Link*lnk = udp->pin(0).next_link()
		 ; (*lnk) != udp->pin(0) ;  lnk = lnk->next_link()) {

	    if (NetNet*sig = dynamic_cast<NetNet*>(lnk->get_obj()))
		  sig->set_ival(lnk->get_pin(), ival);

      }
}

void propinit(Design*des)
{
      des->clear_node_marks();
      while (NetNode*net = des->find_node(&is_sequ_udp)) {
	    net->set_mark();
	    prop_sequdp_output(dynamic_cast<NetUDP*>(net));
      }
}

/*
 * $Log: propinit.cc,v $
 * Revision 1.1  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 */

