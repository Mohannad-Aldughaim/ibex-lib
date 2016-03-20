
#ifndef __EXPR_DATA_H__
#define __EXPR_DATA_H__

#include "ibex_Array.h"
#include "ibex_ExprVisitor.h"

namespace ibex {

class Function;

/**
 * \brief Data associated to a function.
 *
 * Each node of the function expression is decorated
 * with an object of type "T".
 *
 */
template<class T>
class ExprData : private ExprVisitor {
public:

	/**
	 * Initialize this data (of type T) associated to a function.
	 */
	ExprData(Function& f);

	/**
	 * (Does nothing)
	 */
	virtual ~ExprData();

	/**
	 * Get the data associated to the ith node (in prefix order).
	 */
	const T& operator[](int i) const;

	/**
	 * Get the data associated to the ith node (in prefix order).
	 */
	T& operator[](int i);

	/**
	 *  The function
	 */
	Function& f;

	/**
	 *  Data
	 */
	Array<T> data;

	/**
	 * Data of the arguments of the function
	 */
	Array<T> args;

	/**
	 * Data of the root node of the expression
	 */
	T& top;

protected:
	/**
	 * Visit an expression.
	 * Principle: Either this function is overridden or the principle
	 * applies for all direct sub-classes of ExprUnaryOp.
	 */
	virtual T* init(const ExprNode& node) {
		ibex_error("ExprAlgorithm::init: Missing implementation for some node type");
		return NULL;
	}

	/** Visit an indexed expression. */
	virtual T* init(const ExprIndex& e, const T& expr_deco) {
		return init((const ExprNode&) e);
	}

	/** Visit a leaf.*/
	virtual T* init(const ExprLeaf& e) {
		return init((const ExprNode&) e);
	}

	/** Visit a n-ary operator. */
	virtual T* init(const ExprNAryOp& e, const Array<T>& args_deco) {
		return init((const ExprNode&) e);
	}

	/** Visit a binary operator. */
	virtual T* init(const ExprBinaryOp& e, const T& left_deco, const T& right_deco) {
		return init((const ExprNode&) e);
	}

	/** Visit an unary operator. */
	virtual T* init(const ExprUnaryOp& e, const T& expr_deco) {
		return init((const ExprNode&) e);
	}

	/*==================== 0-ary operators =========================*/
	/** Visit a symbol.
	 * By default: call init(const ExprLeaf& e). */
	virtual T* init(const ExprSymbol& e) {
		return init((const ExprLeaf&) e);
	}

	/** Visit a constant.
	 * By default: call init(const ExprLeaf& e). */
	virtual T* init(const ExprConstant& e) {
		return init((const ExprLeaf&) e);
	}

	/*==================== n-ary operators =========================*/
	/** Visit a vector of expressions.
	 * By default: call init(const ExprNAryOp& e). */
	virtual T* init(const ExprVector& e, const Array<T>& args_deco) {
		return init((const ExprNAryOp&) e, args_deco);
	}

	/** Visit a function application.
	 * By default: call init(const ExprNAryOp& e). */
	virtual T* init(const ExprApply& e, const Array<T>& args_deco) {
		return init((const ExprNAryOp&) e, args_deco);
	}

	/** Visit a function Chi.
	 * By default: call init(const ExprNAryOp& e). */
	virtual T* init(const ExprChi& e, const Array<T>& args_deco) {
		return init((const ExprNAryOp&) e, args_deco);
	}

	/*==================== binary operators =========================*/
	/** Visit an addition (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprAdd& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}

	/** Visit an multiplication (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprMul& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}

	/** Visit a subtraction (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprSub& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}

	/** Visit a division (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprDiv& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}

	/** Visit a max  (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprMax& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}

	/** Visit a min (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprMin& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}

	/** Visit a arctan2 (Implementation is not mandatory).
	 * By default: call init(const ExprBinaryOp& e). */
	virtual T* init(const ExprAtan2& e, const T& left_deco, const T& right_deco) {
		return init((const ExprBinaryOp&) e, left_deco, right_deco);
	}


	/*==================== unary operators =========================*/
	/** Visit a minus sign (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprMinus& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a transpose (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprTrans& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a sign (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprSign& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit an abs (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAbs& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a power expression.
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprPower& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a sqr (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprSqr& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a sqrt (Implementation is no
	 * By default: does nothing. t mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprSqrt& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit an exp (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprExp& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a log (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprLog& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a cos (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprCos& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a sin (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprSin& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a tan (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprTan& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a cosh (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprCosh& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a sinh (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprSinh& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a tanh (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprTanh& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a acos (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAcos& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a asin (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAsin& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a atan (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAtan& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a acosh (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAcosh& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a asinh (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAsinh& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

	/** Visit a atanh (Implementation is not mandatory).
	 * By default call init(const ExprUnaryOp&). */
	virtual T* init(const ExprAtanh& e, const T& expr_deco) {
		return init((const ExprUnaryOp&) e, expr_deco);
	}

private:

	template<class Node>
	void __visit_nary(const Node& e) {
		Array<T> args_data(e.nb_args);
		for (int i=0; i<e.nb_args; i++)
			args_data.set_ref(i,data[f.nodes.rank(e.arg(i))]);

		data.set_ref(f.nodes.rank(e), *init(e,args_data));
	}

	template<class Node>
	void __visit_binary(const Node& e) {
		T& left_data=data[f.nodes.rank(e.left)];
		T& right_data=data[f.nodes.rank(e.right)];

		data.set_ref(f.nodes.rank(e), *init(e,left_data,right_data));
	}

	template<class Node>
	void __visit_unary(const Node& e) {
		T& expr_data=data[f.nodes.rank(e.expr)];
		data.set_ref(f.nodes.rank(e), *init(e,expr_data));
	}

	template<class Node>
	void __visit_0ary(const Node& e) {
		data.set_ref(f.nodes.rank(e), *init(e));
	}

	virtual void visit(const ExprIndex& e)   { __visit_unary<ExprIndex>(e); }
	virtual void visit(const ExprVector& e)  { __visit_nary<ExprVector>(e); }
	virtual void visit(const ExprApply& e)   { __visit_nary<ExprApply>(e); }
	virtual void visit(const ExprChi& e)     { __visit_nary<ExprChi>(e); }
	virtual void visit(const ExprAdd& e)     { __visit_binary<ExprAdd>(e); }
	virtual void visit(const ExprMul& e)     { __visit_binary<ExprMul>(e); }
	virtual void visit(const ExprSub& e)     { __visit_binary<ExprSub>(e); }
	virtual void visit(const ExprDiv& e)     { __visit_binary<ExprDiv>(e); }
	virtual void visit(const ExprMax& e)     { __visit_binary<ExprMax>(e); }
	virtual void visit(const ExprMin& e)     { __visit_binary<ExprMin>(e); }
	virtual void visit(const ExprAtan2& e)   { __visit_binary<ExprAtan2>(e); }
	virtual void visit(const ExprMinus& e)   { __visit_unary<ExprMinus>(e); }
	virtual void visit(const ExprTrans& e)   { __visit_unary<ExprTrans>(e); }
	virtual void visit(const ExprSign& e)    { __visit_unary<ExprSign>(e); }
	virtual void visit(const ExprAbs& e)     { __visit_unary<ExprAbs>(e); }
	virtual void visit(const ExprSqr& e)     { __visit_unary<ExprSqr>(e); }
	virtual void visit(const ExprSqrt& e)    { __visit_unary<ExprSqrt>(e); }
	virtual void visit(const ExprExp& e)     { __visit_unary<ExprExp>(e); }
	virtual void visit(const ExprLog& e)     { __visit_unary<ExprLog>(e); }
	virtual void visit(const ExprCos& e)     { __visit_unary<ExprCos>(e); }
	virtual void visit(const ExprSin& e)     { __visit_unary<ExprSin>(e); }
	virtual void visit(const ExprTan& e)     { __visit_unary<ExprTan>(e); }
	virtual void visit(const ExprCosh& e)    { __visit_unary<ExprCosh>(e); }
	virtual void visit(const ExprSinh& e)    { __visit_unary<ExprSinh>(e); }
	virtual void visit(const ExprTanh& e)    { __visit_unary<ExprTanh>(e); }
	virtual void visit(const ExprAcos& e)    { __visit_unary<ExprAcos>(e); }
	virtual void visit(const ExprAsin& e)    { __visit_unary<ExprAsin>(e); }
	virtual void visit(const ExprAtan& e)    { __visit_unary<ExprAtan>(e); }
	virtual void visit(const ExprAcosh& e)   { __visit_unary<ExprAcosh>(e); }
	virtual void visit(const ExprAsinh& e)   { __visit_unary<ExprAsinh>(e); }
	virtual void visit(const ExprAtanh& e)   { __visit_unary<ExprAtanh>(e); }
	virtual void visit(const ExprConstant& e){ __visit_0ary<ExprConstant>(e); }
	virtual void visit(const ExprSymbol& e)  { __visit_0ary<ExprSymbol>(e); }

};

} // end namespace

#include "ibex_Function.h"

namespace ibex {

/* ============================================================================
 	 	 	 	 	 	 	 implementation
  ============================================================================*/

template<class T>
ExprData<T>::ExprData(Function& f) : f(f), data(f.nodes.size()), args(f.nb_arg()), top(data[0]) {

	int n=f.nodes.size();
	for (int ptr=n-1; ptr>=0; ptr--) {
		f.nodes[ptr].acceptVisitor(*this);
	}

	for (int i=0; i<f.nb_arg(); i++) {
		args.set_ref(i,data[i]);
	}
}

template<class T>
ExprData<T>::~ExprData() {
}

template<class T>
inline const T& ExprData<T>::operator[](int i) const {
	return data[i];
}

template<class T>
inline T& ExprData<T>::operator[](int i) {
	return data[i];
}

} /* namespace ibex */

#endif /* __EXPR_DATA_H__ */
