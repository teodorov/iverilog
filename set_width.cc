/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: set_width.cc,v 1.2 1999/09/23 02:27:50 steve Exp $"
#endif

/*
 * This file contains set_width methods for the various NetExpr
 * classes. The set_width method is used by elaboration to ask the
 * expression to resize itself. If the expression can't, then the
 * set_width method will return false and the caller will arrange for
 * whatever is needed to deal with the size mismatch.
 */
# include  "netlist.h"
# include  <typeinfo>


bool NetExpr::set_width(unsigned w)
{
      cerr << typeid(*this).name() << ": set_width(unsigned) "
	    "not implemented." << endl;
      expr_width(w);
      return false;
}

bool NetEBinary::set_width(unsigned w)
{
      bool flag = true;
      switch (op_) {

	  case 'l': // left shift  (<<)
	  case 'r': // right shift (>>)
	    flag = left_->set_width(w);
	    expr_width(w);
	    break;

	      /* The default rule is that the operands of the binary
		 operator might as well use the same width as the
		 output from the binary operation. */
	  default:
	    expr_width(left_->expr_width() > right_->expr_width()
			  ? left_->expr_width() : right_->expr_width());
	    cerr << "NetEBinary::set_width(): Using default for " <<
		  op_ << "." << endl;
	    flag = false;

	  case '%':
	  case '/':
	    flag = left_->set_width(w) && flag;
	    flag = right_->set_width(w) && flag;
	    expr_width(w);
	    break;
      }
      return flag;
}

/*
 * The bitwise logical operators have operands the same size as the
 * result. Anything else is a mess.
 */
bool NetEBAdd::set_width(unsigned w)
{
      bool flag = true;

      if (left_->expr_width() > right_->expr_width())
	    right_->set_width(left_->expr_width());
      else
	    left_->set_width(right_->expr_width());

      if (left_->expr_width() == w)
	    expr_width(w);
      else if (left_->expr_width() == (w-1))
	    expr_width(w);
      else
	    flag = false;

      return flag;
}

/*
 * The bitwise logical operators have operands the same size as the
 * result. Anything else is a mess.
 */
bool NetEBBits::set_width(unsigned w)
{
      bool flag = true;

      flag = left_->set_width(w) && flag;
      flag = right_->set_width(w) && flag;
      expr_width(w);

      return flag;
}

/*
 * Comparison operators allow the subexpressions to have
 * their own natural width, but the comparison operator result has a
 * fixed width of 1.
 */
bool NetEBComp::set_width(unsigned w)
{
      bool flag = true;
      return (w == 1);
}

bool NetEBLogic::set_width(unsigned w)
{
      bool flag;
      flag = left_->set_width(right_->expr_width());
      if (!flag)
	    flag = right_->set_width(left_->expr_width());
      return (w == 1);
}

/*
 * Add up the widths from all the expressions that are concatenated
 * together. This is the width of the expression, tough luck if you
 * want it otherwise.
 *
 * If during the course of elaboration one of the sub-expressions is
 * broken, then don't count it in the width. This doesn't really
 * matter because the null expression is indication of an error and
 * the compiler will not go beyond elaboration.
 */
bool NetEConcat::set_width(unsigned w)
{
      unsigned sum = 0;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    if (parms_[idx] != 0)
		  sum += parms_[idx]->expr_width();

      sum *= repeat_;
      expr_width(sum);
      if (sum != w) return false;
      return true;
}

bool NetEConst::set_width(unsigned w)
{
      if (w > value_.len())
	    return false;

      assert(w <= value_.len());
      value_ = verinum(value_, w);
      expr_width(w);
      return true;
}

bool NetEMemory::set_width(unsigned w)
{
      if (w != mem_->width())
	    return false;

      expr_width(w);
      return true;
}

bool NetEParam::set_width(unsigned)
{
      return false;
}

/*
 * The signal should automatically pad with zeros to get to th desired
 * width. Do not allow signal bits to be truncated, however.
 */
bool NetESignal::set_width(unsigned w)
{
      if (w != pin_count())
	    return false;

      return true;
}

bool NetESubSignal::set_width(unsigned w)
{
      if (w != 1) return false;
      return true;
}

bool NetETernary::set_width(unsigned w)
{
      bool flag = true;
      flag = flag && true_val_->set_width(w);
      flag = flag && false_val_->set_width(w);
      expr_width(true_val_->expr_width());
      return flag;
}

/*
 * XXXX FIX ME: For now, just take whatever the caller says as my
 * width. What I really need to do is note the width of the output
 * parameter of the function definition and take that into account.
 */
bool NetEUFunc::set_width(unsigned wid)
{
      expr_width(wid);
      return true;
}

bool NetEUnary::set_width(unsigned w)
{
      bool flag = true;
      switch (op_) {
	  case '~':
	  case '-':
	    flag = expr_->set_width(w);
	    break;
	  case '&':
	  case '!':
	    if (w != 1) {
		  flag = false;
	    }
	    break;
	  default:
	    flag = false;
	    break;
      }
      expr_width(w);
      return flag;
}


/*
 * $Log: set_width.cc,v $
 * Revision 1.2  1999/09/23 02:27:50  steve
 *  comparison parameter width is self determined.
 *
 * Revision 1.1  1999/09/23 00:21:55  steve
 *  Move set_width methods into a single file,
 *  Add the NetEBLogic class for logic expressions,
 *  Fix error setting with of && in if statements.
 *
 */

