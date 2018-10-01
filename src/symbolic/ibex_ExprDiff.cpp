//============================================================================
//                                  I B E e
// File        : ibex_ExprDiff.cpp
// Author      : Gilles Chabert
// Diffright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : Feb 25, 2013
// Last Update : Feb 25, 2013
//============================================================================

#include "ibex_ExprDiff.h"
#include "ibex_ExprCopy.h"
#include "ibex_ExprSubNodes.h"
#include "ibex_Expr.h"
#include "ibex_ExprSimplify.h"

using namespace std;

namespace ibex {

#define ONE          ExprConstant::new_scalar(1.0)
#define ZERO         ExprConstant::new_scalar(0.0)
#define ALL_REALS    ExprConstant::new_scalar(Interval::ALL_REALS)

ExprDiffException::ExprDiffException(const std::string& msg) : msg(msg) {

}

std::ostream& operator<< (std::ostream& os, const ExprDiffException& e) {
	return os << e.msg;
}

ExprDiff::ExprDiff() : old_symbols(NULL), new_symbols(NULL) {

}

ExprDiff::ExprDiff(const Array<const ExprSymbol>& old_symbols, const Array<const ExprSymbol>& new_symbols) :
		old_symbols(&old_symbols), new_symbols(&new_symbols) {
}

void ExprDiff::add_grad_expr(const ExprNode& node, const ExprNode& _expr_) {

	if (grad.found(node))
		grad[node]= & (*grad[node] +_expr_);
	else
		// not found means "zero"
		grad.insert(node, &_expr_);
}

const ExprNode& ExprDiff::diff(const ExprNode& y, const Array<const ExprSymbol>& x) {
	const ExprNode* res;

	if (y.dim.is_scalar()) {
		res=&gradient(y,x); // already simplified
	} else if (y.dim.is_vector()) {
		if (y.dim.type()==Dim::ROW_VECTOR)
			ibex_warning("differentiation of a function returning a row vector (considered as a column vector)");

		int m=y.dim.vec_size();
		Array<const ExprNode> a(m);

		for (int i=0; i<m; i++) { // y.dim.vec_size() == vec->nb_args()
			const ExprNode& argi=y[i]; // temporary node creation
			a.set_ref(i,gradient(argi,x));
			delete &argi;
		}
		res=&ExprVector::new_col(a);
	} else {
		throw ExprDiffException("differentiation of matrix-valued functions");
	}

	return res->simplify();
}

const ExprNode& ExprDiff::gradient(const ExprNode& y, const Array<const ExprSymbol>& x) {

	grad.clean();
	groots.clear();

	ExprSubNodes nodes(y);
	//cout << "y =" << y;
	int n=y.size;
	int nb_var=0;
	for (int i=0; i<x.size(); i++) {
		nb_var += x[i].dim.size();
	}

	add_grad_expr(nodes[0],ONE);

	// visit nodes in topological order
	for (int i=0; i<n; i++) {
		visit(nodes[i]);
	}

	Array<const ExprNode> dX(nb_var);

	// =============== set null derivative for missing variables ===================
	// note: we have to make the association with grad[old_x[i]] because this map is
	// cleared after.
	for (int i=0; i<x.size(); i++) {

		if (!grad.found(x[i]))
			// this symbol does not appear in the expression -> null derivative
			grad.insert(x[i], &ExprConstant::new_matrix(Matrix::zeros(x[i].dim.nb_rows(),x[i].dim.nb_cols())));
	}

	{   // =============== build dX ===================
		int k=0; // count components of x

		for (int i=0; i<x.size(); i++) {

	    	const Dim& d=x[i].dim;
	    	//cout << "grad % " << old_x[i].name << " : " << *grad[old_x[i]] << endl;
			switch (d.type()) {
			case Dim::SCALAR:
				dX.set_ref(k,*grad[x[i]]);
				groots.push_back(&dX[k]);
				k++;
				break;
			case Dim::ROW_VECTOR:
			case Dim::COL_VECTOR:
				{
					for (int j=0; j<d.vec_size(); j++) {
						dX.set_ref(k,(*grad[x[i]])[j]);
						groots.push_back(&dX[k]);
						k++;
					}
				}
			break;
			case Dim::MATRIX:
			    {
			    	for (int j=0; j<d.nb_rows(); j++)
			    		for (int j2=0; j2<d.nb_cols(); j2++) {
			    			dX.set_ref(k,(*grad[x[i]])[DoubleIndex::one_elt(d,j,j2)]);
			    			groots.push_back(&dX[k]);
			    			k++;
			    		}
			    }
			    break;
			}
		}
		assert(k==nb_var);
	}

//	cout << "(";
//	for (int k=0; k<old_x_vars.nb_var; k++) cout << dX[k] << " , ";
//	cout << ")" << endl;

    // dX.size()==1 is the univariate case (the node df must be scalar)
	const ExprNode& df=dX.size()==1? dX[0] : ExprVector::new_(dX,ExprVector::ROW);

	// ====== for cleanup =====================================
	NodeMap<bool> leaks;
	// =========================================================

	if (new_symbols!=NULL) {
		// Note: it is better to proceed in this way: (1) differentiate
		// and (2) copy the expression for two reasons
		// 1-we can eliminate the constant expressions such as (1*1)
		//   generated by the differentiation (thanks to simplification)
		// 2-the dead branches corresponding to the partial derivative
		//   w.r.t. ExprConstant leaves will be deleted properly since
		//   we delete all created nodes that do not belong to the original
		//   expression (see "other_nodes").

		const ExprNode& result=ExprCopy().copy(*old_symbols, *new_symbols, df);

		// ------------------------- CLEANUP -------------------------
		// cleanup(df,true); // don't! some nodes are shared with y

		// don't! some grad are references to nodes of y!
		//	for (int i=0; i<n; i++)
		//	  delete grad[*nodes[i]];

		ExprSubNodes gnodes(groots);

		for (int i=0; i<gnodes.size(); i++) {
			if (!nodes.found(gnodes[i])        // if it is not in the expression
					&& !leaks.found(gnodes[i]) // and not yet collected
			) {
				leaks.insert(gnodes[i],true);
			}
		}

		for (IBEX_NODE_MAP(bool)::const_iterator it=leaks.begin(); it!=leaks.end(); it++) {
			delete it->first;
		}

		if (dX.size()>1) delete &df; // delete the Vector node

		//cout << "   ---> grad:" << result << endl;
		return result;
	} else {
		ExprSubNodes df_nodes(df);

		// Destroy also the leaking nodes of the original expression.
		// Note: since the original expression will anyway be partly destroyed
		// (as some nodes belong to df which is going to be simplified) it is safer
		// to destroy all the leaking nodes.
		groots.push_back(&y);

		ExprSubNodes gnodes(groots);

		// Destroy the dead branches (nodes created by
		// the diff process, but unused) + leaking nodes of "y"
		for (int i=0; i<gnodes.size(); i++) {
			if (!df_nodes.found(gnodes[i])           // if it not in the result of differentiation
					&& !leaks.found(gnodes[i]) // and not yet collected
			) {
				leaks.insert(gnodes[i],true);
			}
		}

		for (IBEX_NODE_MAP(bool)::const_iterator it=leaks.begin(); it!=leaks.end(); it++) {
			delete it->first;
		}

		return df;
	}
}

void ExprDiff::visit(const ExprNode& e) {
	e.acceptVisitor(*this);
}

void ExprDiff::visit(const ExprIndex& i) {

	if (i.index.all_rows() && i.index.all_cols()) {
		add_grad_expr(i.expr, *grad[i]);
		return;
	}

	vector<const ExprNode*> row_vec;

	int n=i.expr.dim.nb_rows();
	int m=i.index.first_col();

	if (m>0) { // fill with zeros the left part of the matrix expression
		// will be automatically transformed to a vector (if n=1) or a scalar (n=m=1)
		row_vec.push_back(&ExprConstant::new_matrix(Matrix::zeros(n,m)));
	}

	n=i.index.first_row();
	m=i.index.nb_cols();

	vector<const ExprNode*> col_vec;
	if (n>0) { // fill with zeros on the top
		// will be automatically transformed to a vector (if n=1) or a scalar (n=m=1)
		col_vec.push_back(&ExprConstant::new_matrix(Matrix::zeros(n,m)));
	}

	col_vec.push_back(grad[i]);

	n=i.expr.dim.nb_rows() - i.index.last_row() -1;
	if (n>0) { // fill with zeros on the right
		col_vec.push_back(&ExprConstant::new_matrix(Matrix::zeros(n,m)));
	}

	if (col_vec.size()==1) {
		assert(i.index.all_rows());
		row_vec.push_back(col_vec.back());
	} else {
		row_vec.push_back(&ExprVector::new_(col_vec,ExprVector::COL));
	}

	n=i.expr.dim.nb_rows();
	m=i.expr.dim.nb_cols() - i.index.last_col() -1;
	if (m>0) { // fill with zeros on the bottom
		row_vec.push_back(&ExprConstant::new_matrix(Matrix::zeros(n,m)));
	}

	if (row_vec.size()==1) {
		assert(i.index.all_cols());
		add_grad_expr(i.expr,*row_vec.back());
	} else {
		add_grad_expr(i.expr, ExprVector::new_(row_vec,ExprVector::ROW));
	}
}

void ExprDiff::visit(const ExprSymbol& x) {
	// note: if x is a vector/matrix, grad[x]
	// will not be a root (only the gradients
	// of its components will), unless it
	// is not a symbol in "x" (a symbol we
	// diff). So, to simplify, we add it in the
	// root array.
	groots.push_back(grad[x]);
}

void ExprDiff::visit(const ExprConstant& c) {
	groots.push_back(grad[c]);
}

void ExprDiff::visit(const ExprVector& e) {
	int j=0;
	int n;
	DoubleIndex idx;
	for (int i=0; i<e.nb_args; i++) {
		if (e.row_vector()) {
			n=e.arg(i).dim.nb_cols();
			idx=DoubleIndex::cols(e.dim,j,j+n-1);
		} else  {
			n=e.arg(i).dim.nb_rows();
			idx=DoubleIndex::rows(e.dim,j,j+n-1);
		}
		j+=n;
		//cout << "i=" << i << " idx=" << idx << " grad[idx]=" << (*grad[e])[idx] << endl;
		add_grad_expr(e.arg(i), (*grad[e])[idx]); // needs a call to simplify
	}
}

void ExprDiff::visit(const ExprApply& e) {

	/* ******* TODO:old code ********/

	const Function& df=e.func.diff();
	int k=0;
	const ExprNode& gradf=df(e.args);
	for (int i=0; i<e.nb_args; i++) {
		switch (e.arg(i).dim.type()) {
		case Dim::SCALAR:
			if (e.nb_args==1)
				add_grad_expr(e.arg(i), gradf*(*grad[e])); // to avoid a useless [0] index
			else
				add_grad_expr(e.arg(i), gradf[k++]*(*grad[e]));
			break;
		case Dim::ROW_VECTOR:
		case Dim::COL_VECTOR:
		{
			int n=e.arg(i).dim.vec_size();
			Array<const ExprNode> tab(n);
			for (int j=0; j<n; j++) tab.set_ref(j,gradf[k++]*(*grad[e]));
			add_grad_expr(e.arg(i), ExprVector::new_(tab,e.arg(i).dim.type()==Dim::ROW_VECTOR? ExprVector::ROW : ExprVector::COL));
		}
		break;
		case Dim::MATRIX:
		{
			throw ExprDiffException("diff with function apply and matrix arguments");

			// In case this argument is a ExprVector, we keep for the matrix representing the "partial gradient"
			// wrt to this argument the same structure, that is, if this argument is a "row of column vectors"
			// (versus a "column of row vectors") the partial gradient is also a "row of column vectors".
			// It is a necessity because, further, the i^th argument of the argument will be associated
			// to the i^th argument of the gradient (see visit(ExprVector&).
//			const ExprVector* vec=dynamic_cast<const ExprVector*>(&e.arg(i));
//			if (vec!=NULL) {
//				int m=e.arg(i).dim.nb_rows();
//				int n=e.arg(i).dim.nb_cols();
//				if (Array<const ExprNode> rows(m);
//				for (int i=0; i<m; i++) {
//					Array<const ExprNode> col(n);
//					for (int j=0; j<n; j++) col.set_ref(j,gradf[k++]*grad[e]);
//					rows.set_ref(i, ExprVector::new_(col,ExprVector::COL));
//				}
//				add_grad_expr(e.arg(i), ExprVector::new_(rows,ExprVector::ROW));
//			}
		}
		break;
		}
	}

}

void ExprDiff::visit(const ExprChi& e) {
	throw ExprDiffException("symbolic differentiation with chi");
}


void ExprDiff::visit(const ExprMul& e)   {
	if (e.left.dim.is_scalar()) {
		if (!e.right.dim.is_matrix()) {
			add_grad_expr(e.left,  (*grad[e])*e.right); // S*S or V*V
		} else {
			for (int i=0; i<e.right.dim.nb_rows(); i++) {
				DoubleIndex idx=DoubleIndex::one_row(e.right.dim,i);
				add_grad_expr(e.left, (*grad[e])[idx]*(e.right[idx]));
			}
		}
		add_grad_expr(e.right, e.left*(*grad[e])); // S*S or S*V or S*M
	} else if (e.left.dim.is_vector()) {
		if (e.right.dim.is_vector()) {
			add_grad_expr(e.left,  (*grad[e])*transpose(e.right)); // S*V
			add_grad_expr(e.right, (*grad[e])*transpose(e.left));  // S*V
		} else {
			assert(e.right.dim.is_matrix());
			add_grad_expr(e.left,  (*grad[e])*transpose(e.right)); // (M*V)'
			add_grad_expr(e.right, (transpose(e.left))*(*grad[e])); // V'*V
		}
	} else {
		assert(e.left.dim.is_matrix());
		add_grad_expr(e.left, *grad[e]*transpose(e.right)); // V*V' or M*M
		add_grad_expr(e.right,transpose(e.left)*(*grad[e])); // M*V or M*M
	}
}

void ExprDiff::visit(const ExprAdd& e)   { add_grad_expr(e.left,  *grad[e]);
                                           add_grad_expr(e.right, *grad[e]); }
void ExprDiff::visit(const ExprSub& e)   { add_grad_expr(e.left,  *grad[e]);
                                           add_grad_expr(e.right, -*grad[e]); }
void ExprDiff::visit(const ExprDiv& e)   { add_grad_expr(e.left,  *grad[e]/e.right);
		                                   add_grad_expr(e.right, -((*grad[e])*e.left/sqr(e.right))); }
void ExprDiff::visit(const ExprMax& e)   { add_grad_expr(e.left, (*grad[e])*chi(e.right-e.left, ONE, ZERO));
										   add_grad_expr(e.right,(*grad[e])*chi(e.left-e.right, ONE, ZERO)); }
void ExprDiff::visit(const ExprMin& e)   { add_grad_expr(e.left, (*grad[e])*chi(e.left-e.right, ONE, ZERO));
                                           add_grad_expr(e.right,(*grad[e])*chi(e.right-e.left, ONE, ZERO)); }
void ExprDiff::visit(const ExprAtan2& e) {
    add_grad_expr(e.left,  e.right / (sqr(e.left) + sqr(e.right)) * *grad[e]);
    add_grad_expr(e.right, - e.left / (sqr(e.left) + sqr(e.right)) * *grad[e]);
}

void ExprDiff::visit(const ExprPower& e) {
	add_grad_expr(e.expr,Interval(e.expon)*pow(e.expr,e.expon-1)*(*grad[e]));
}

void ExprDiff::visit(const ExprGenericUnaryOp& e) {
	                                       add_grad_expr(e.expr, (*grad[e])*(e.symb_diff(e.expr))); }
void ExprDiff::visit(const ExprMinus& e) { add_grad_expr(e.expr, -*grad[e]); }
void ExprDiff::visit(const ExprTrans& e) { add_grad_expr(e.expr, transpose(*grad[e])); }
void ExprDiff::visit(const ExprSign& e)  { add_grad_expr(e.expr, (*grad[e])*chi(abs(e.expr),ALL_REALS,ZERO)); }
void ExprDiff::visit(const ExprAbs& e)   { add_grad_expr(e.expr, (*grad[e])*sign(e.expr)); }
void ExprDiff::visit(const ExprSqr& e)   { add_grad_expr(e.expr, (*grad[e])*Interval(2.0)*e.expr); }
void ExprDiff::visit(const ExprSqrt& e)  { add_grad_expr(e.expr, (*grad[e])*Interval(0.5)/sqrt(e.expr)); }
void ExprDiff::visit(const ExprExp& e)   { add_grad_expr(e.expr, (*grad[e])*exp(e.expr)); }
void ExprDiff::visit(const ExprLog& e)   { add_grad_expr(e.expr, (*grad[e])/e.expr ); }
void ExprDiff::visit(const ExprCos& e)   { add_grad_expr(e.expr,-(*grad[e])*sin(e.expr) ); }
void ExprDiff::visit(const ExprSin& e)   { add_grad_expr(e.expr, (*grad[e])*cos(e.expr) ); }
void ExprDiff::visit(const ExprTan& e)   { add_grad_expr(e.expr, (*grad[e])*(1.0+sqr(tan(e.expr)))); }
void ExprDiff::visit(const ExprCosh& e)  { add_grad_expr(e.expr, (*grad[e])*sinh(e.expr)); }
void ExprDiff::visit(const ExprSinh& e)  { add_grad_expr(e.expr, (*grad[e])*cosh(e.expr)); }
void ExprDiff::visit(const ExprTanh& e)  { add_grad_expr(e.expr, (*grad[e])*(1.0-sqr(tanh(e.expr)))); }
void ExprDiff::visit(const ExprAcos& e)  { add_grad_expr(e.expr,-(*grad[e])/sqrt(1.0-sqr(e.expr))); }
void ExprDiff::visit(const ExprAsin& e)  { add_grad_expr(e.expr, (*grad[e])/sqrt(1.0-sqr(e.expr))); }
void ExprDiff::visit(const ExprAtan& e)  { add_grad_expr(e.expr, (*grad[e])/(1.0+sqr(e.expr))); }
void ExprDiff::visit(const ExprAcosh& e) { add_grad_expr(e.expr, (*grad[e])/sqrt(sqr(e.expr) -1.0)); }
void ExprDiff::visit(const ExprAsinh& e) { add_grad_expr(e.expr, (*grad[e])/sqrt(1.0+sqr(e.expr))); }
void ExprDiff::visit(const ExprAtanh& e) { add_grad_expr(e.expr, (*grad[e])/(1.0-sqr(e.expr))); }

} // end namespace ibex
