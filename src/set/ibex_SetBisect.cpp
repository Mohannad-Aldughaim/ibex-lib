//============================================================================
//                                  I B E X                                   
// File        : ibex_SetBisect.cpp
// Author      : Gilles Chabert
// Copyright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : 13 juil. 2014
//============================================================================

#include "ibex_SetBisect.h"
#include "ibex_SetLeaf.h"
#include <stack>
#include <utility>

using namespace std;

// =========== shortcuts ==================
#define IN         __IBEX_IN__
#define OUT        __IBEX_OUT__
#define UNK        __IBEX_UNK__
#define UNK_IN     __IBEX_UNK_IN__
#define UNK_OUT    __IBEX_UNK_OUT__
#define UNK_IN_OUT __IBEX_UNK_IN_OUT__
// ========================================

namespace ibex {

SetBisect::SetBisect(int var, double pt, SetNode* left, SetNode* right) : SetNode(left->status | right->status), var(var), pt(pt), left(left), right(right) {
	// a bisectNode with two subnodes of same status IN or OUT should not exist
	// (automatically compacted as a leaf node) but two subnodes IN_TMP can be
	// created by a leaf with IN_TMP status (when it auto-splits in inter function)
	//assert(left->status>=UNK || left->status!=right->status);
}

SetBisect::SetBisect(int var, double pt) : SetNode(UNK), var(var), pt(pt), left(NULL), right(NULL) {

}

SetBisect::~SetBisect() {
	delete left;
	delete right;
}

bool SetBisect::is_leaf() const {
	return false;
}

SetNode* SetBisect::sync(const IntervalVector& nodebox, const IntervalVector& x, NodeType x_status, double eps) {
	if (x_status==UNK || !nodebox.intersects(x)) {
		return this;
	}
	else if (nodebox.is_subset(x)) {
		if (x_status==IN) {
			if (!possibly_contains_in(status)) throw NoSet();
			else {
				delete this; // warning: suicide
				return new SetLeaf(IN);
			}
		}
		else if (x_status==OUT) {
			if (!possibly_contains_out(status)) throw NoSet();
			else {
				delete this; // warning: suicide
				return new SetLeaf(OUT);
			}
		}
		else if (certainly_contains_in(x_status) && !possibly_contains_in(status) && nodebox==x) throw NoSet();
		else if (certainly_contains_out(x_status) && !possibly_contains_out(status) && nodebox==x) throw NoSet();
		else // x_status >= UNK
			return this;
	} else {
		left = left->sync(left_box(nodebox), x, x_status, eps);
		right = right->sync(right_box(nodebox), x, x_status, eps);
		// status of children may have changed --> try merge
		return try_merge();
	}
}

SetNode* SetBisect::sync_rec(const IntervalVector& nodebox, Sep& sep, const IntervalVector& targetbox, double eps) {
	left = left->sync(left_box(nodebox), sep, targetbox, eps);
	right = right->sync(right_box(nodebox), sep, targetbox, eps);
	// status of children may have changed --> try merge
	return try_merge();
}


SetNode* SetBisect::inter(const IntervalVector& nodebox, const IntervalVector& x, NodeType x_status, double eps) {
	// certainly_contains_out in comment because does not take into account IN_TMP
	if (x_status==IN)
		return this;
	else if ((x_status==OUT /*|| !certainly_contains_out(status)*/) && nodebox.is_subset(x)) {
		delete this; // warning: suicide
		return new SetLeaf(OUT);
	} else {
		left = left->inter(left_box(nodebox), x, x_status, eps);
		right = right->inter(right_box(nodebox), x, x_status, eps);
		// status of children may have changed --> try merge
		return try_merge();
	}
}

SetNode* SetBisect::inter_rec(const IntervalVector& nodebox, Sep& sep, const IntervalVector& targetbox, double eps) {
	left = left->inter(left_box(nodebox), sep, targetbox, eps);
	right = right->inter(right_box(nodebox), sep, targetbox, eps);
	// status of children may have changed --> try merge or update status
	return try_merge();
}

SetNode* SetBisect::union_(const IntervalVector& nodebox, const IntervalVector& x, NodeType x_status, double eps) {
	assert(x_status<=UNK);

	if (x_status>IN) {
		return this;
	}

    if (nodebox.is_subset(x)) {
		delete this; // warning: suicide
		return new SetLeaf(IN);
	} else {
		left = left->union_(left_box(nodebox), x, x_status, eps);
		right = right->union_(right_box(nodebox), x, x_status, eps);
		// status of children may have changed --> try merge
		return try_merge();
	}
}

void SetBisect::visit_leaves(leaf_func func, const IntervalVector& nodebox) const {
	left->visit_leaves(func, left_box(nodebox));
	right->visit_leaves(func, right_box(nodebox));
}

void SetBisect::print(ostream& os, const IntervalVector& nodebox, int shift) const {
	for (int i=0; i<shift; i++) os << ' ';
	os << "* " << nodebox << endl;
	left->print(os, left_box(nodebox), shift+2);
	right->print(os, right_box(nodebox), shift+2);
}

IntervalVector SetBisect::left_box(const IntervalVector& nodebox) const {
	IntervalVector leftbox(nodebox);
	assert (nodebox[var].contains(pt));
	leftbox[var] =Interval(nodebox[var].lb(),pt);
	return leftbox;
}

IntervalVector SetBisect::right_box(const IntervalVector& nodebox) const {
	IntervalVector rightbox(nodebox);
	assert (nodebox[var].contains(pt));
	rightbox[var]=Interval(pt,nodebox[var].ub());
	return rightbox;
}

SetNode* SetBisect::try_merge() {
	// note: we cannot merge nodes with status like UNK_IN
	// because we would lose all the information of the subtree!
	if (left->status<=UNK && left->status==right->status) {
		NodeType s=left->status;
		delete this;
		return new SetLeaf(s);
	} else {
		status = left->status | right->status;
		return this;
	}
}

} // namespace ibex
