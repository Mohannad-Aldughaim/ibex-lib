//============================================================================
//                                  I B E X                                   
// File        : Fix-point of a contractor
// Author      : Gilles Chabert
// Copyright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : May 01, 2012
// Last Update : May 01, 2012
//============================================================================

#include "ibex_CtcFixPoint.h"

namespace ibex {

/*! Default composition ratio. */
const double CtcFixPoint::default_ratio = 0.1;

CtcFixPoint::CtcFixPoint(Ctc& ctc, double ratio) : Ctc(ctc.nb_var), ctc(ctc), ratio(ratio) {
}

CtcFixPoint::~CtcFixPoint(){
}

void CtcFixPoint::contract(IntervalVector& box) {

	IntervalVector old_box(box);
	BitSet flags(BitSet::empty(Ctc::NB_OUTPUT_FLAGS));
	BitSet impact(BitSet::all(nb_var));

	do {
		old_box=box;
		ctc.contract(box,impact,flags);
	} while ((!flags[FIXPOINT])&&(!flags[INACTIVE])&&(old_box.rel_distance(box)>ratio));

	if (flags[FIXPOINT]) set_flag(FIXPOINT);
	if (flags[INACTIVE]) set_flag(INACTIVE);
}

} // end namespace ibex
