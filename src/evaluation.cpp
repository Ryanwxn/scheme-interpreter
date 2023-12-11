#include "Def.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Value Let::eval(Assoc &env) {}

Value Lambda::eval(Assoc &env) {}

Value Apply::eval(Assoc &e) {}

Value Letrec::eval(Assoc &env) {}

Value Var::eval(Assoc &e) {
    return find(this -> x, e);
}

Value Fixnum::eval(Assoc &e) {
    return IntegerV(this -> n);
}

Value If::eval(Assoc &e) {
    Value cond = this -> cond -> eval(e);
    if(cond -> v_type == V_BOOL) {
        Boolean *condBool = dynamic_cast<Boolean*>(cond.get());
        if(condBool -> b) return this -> conseq -> eval(e);
        else return this -> alter -> eval(e);
    }
    else return this -> conseq -> eval(e);
}

Value True::eval(Assoc &e) {
    return BooleanV(true);
}

Value False::eval(Assoc &e) {
    return BooleanV(false);
}

Value Begin::eval(Assoc &e) {
    int exprNum = this -> es.size();
    Value tmp = NullV();
    for(int i = 0; i < exprNum; ++i)
        tmp = this -> es[i] -> eval(e);
    return tmp;
}

Value syntaxToValue(const Syntax &syntax) {}

Value Quote::eval(Assoc &e) {
    return syntaxToValue(this -> s);
}

Value MakeVoid::eval(Assoc &e) {
    return VoidV();
}

Value Exit::eval(Assoc &e) {
    return TerminateV();
}

Value Binary::eval(Assoc &e) {
    Value rand1V = this -> rand1 -> eval(e);
    Value rand2V = this -> rand2 -> eval(e); 
    return this -> evalRator(rand1V, rand2V);
}

Value Unary::eval(Assoc &e) {}

Value Mult::evalRator(const Value &rand1, const Value &rand2) {}

Value Plus::evalRator(const Value &rand1, const Value &rand2) {}

Value Minus::evalRator(const Value &rand1, const Value &rand2) {}

Value Less::evalRator(const Value &rand1, const Value &rand2) {}

Value LessEq::evalRator(const Value &rand1, const Value &rand2) {}

Value Equal::evalRator(const Value &rand1, const Value &rand2) {}

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) {}

Value Greater::evalRator(const Value &rand1, const Value &rand2) {}

Value IsEq::evalRator(const Value &rand1, const Value &rand2) {}

Value Cons::evalRator(const Value &rand1, const Value &rand2) {}

Value IsBoolean::evalRator(const Value &rand) {}

Value IsFixnum::evalRator(const Value &rand) {}

Value IsNull::evalRator(const Value &rand) {}

Value IsPair::evalRator(const Value &rand) {}

Value IsProcedure::evalRator(const Value &rand) {}

Value IsSymbol::evalRator(const Value &rand) {}

Value Not::evalRator(const Value &rand) {}

Value Car::evalRator(const Value &rand) {}

Value Cdr::evalRator(const Value &rand) {}