//============================================================================
//                                  I B E X                                   
// File        : ibex_CtcNotIn.h
// Author      : Gilles Chabert
// Copyright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : Feb 24, 2013
// Last Update : Feb 24, 2013
//============================================================================

#ifndef __IBEX_CTC_NOT_IN_H__
#define __IBEX_CTC_NOT_IN_H__

#include "ibex_Ctc.h"
#include "ibex_Function.h"

namespace ibex {

/**
 * \ingroup contractor wrt f(x) not-in [y]
 *
 * TODO: should this class become obsolete with the addition
 *       of disjunctions in the language?
 */
class CtcNotIn : public Ctc {
public:
	/**
	 * \brief Build the contractor for "f(x) not-in [y]".
	 */
	CtcNotIn(Function& f, const Interval& y);

	/**
	 * \brief Build the contractor for "f(x) not-in [y]".
	 */
	CtcNotIn(Function& f, const IntervalVector& y);

	/**
	 * \brief Build the contractor for "f(x) not-in [y]".
	 */
	CtcNotIn(Function& f, const IntervalMatrix& y);

	/**
	 * \brief Contract the box.
	 */
	virtual void contract(IntervalVector& box);

protected:
	Function& f;
	Domain d1; // (-oo, lb(y))
	Domain d2; // (ub(y), +oo)
	HC4Revise hc4r;

};

} // end namespace ibex

#endif // __IBEX_CTC_NOT_IN_H__
