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

Value Let::eval(Assoc &env) {
    Assoc bodyEnv = env;
    for(auto &[argName, argValue] : this -> bind)
        bodyEnv = extend(argName, argValue -> eval(env), bodyEnv);
    std::cout << "Let" << std::endl;
    return this -> body -> eval(bodyEnv);
}

Value Lambda::eval(Assoc &env) {
    std::vector<std::string> paras;
    for(auto &para : this -> x)
        paras.push_back(para);
    std::cout << "Lambda" << std::endl;
    return ClosureV(paras, this -> e, env);
}

Value Apply::eval(Assoc &e) {
    Value func = this -> rator -> eval(e);
    if(func -> v_type != V_PROC)
        throw RuntimeError("<Evaluation> In apply : the first value is not a closure.");
    Closure* clos = dynamic_cast<Closure*>(func.get());
    Assoc bodyEnv = clos -> env;
    if(this -> rand.size() != clos -> parameters.size())
        throw RuntimeError("<Evaluation> In apply : the number of parameters doesn't match.");
    int paraNum = this -> rand.size();
    for(int i = 0; i < paraNum; ++i)
        bodyEnv = extend(clos -> parameters[i], this -> rand[i] -> eval(e), bodyEnv);
    std::cout << "Apply" << std::endl;
    return clos -> e -> eval(bodyEnv);
}

Value Letrec::eval(Assoc &env) {
    Assoc virtualEnv = env;
    std::cout << "Letrec0" << std::endl;
    for(auto &[argName, argValue] : this -> bind)
        virtualEnv = extend(argName, Value(nullptr), virtualEnv);
    Assoc bodyEnv = env;
    std::cout << "Letrec1" << std::endl;
    for(auto &[argName, argValue] : this -> bind)
        bodyEnv = extend(argName, argValue -> eval(virtualEnv), bodyEnv);
    std::cout << "Letrec" << std::endl;
    return this -> body -> eval(bodyEnv);
}

Value Var::eval(Assoc &e) {
    Value result = find(this -> x, e);
    if(result.get() == nullptr)
        throw RuntimeError("<Evaluation> Undefined variable.");
    return result;
}

Value Fixnum::eval(Assoc &e) {
    return IntegerV(this -> n);
}

Value If::eval(Assoc &e) {
    Value cond = this -> cond -> eval(e);
    if(cond -> v_type == V_BOOL)
        if(!(dynamic_cast<Boolean*>(cond.get()) -> b)) return this -> alter -> eval(e);
    return this -> conseq -> eval(e);
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

Value Quote::eval(Assoc &e) {
    return this -> s -> eval();
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

Value Unary::eval(Assoc &e) {
    Value randV = this -> rand -> eval(e);
    return this -> evalRator(randV);
}

Value Mult::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In Mult.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return IntegerV(rand1V * rand2V);
}

Value Plus::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In Plus.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return IntegerV(rand1V + rand2V);  
}

Value Minus::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In Minus.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return IntegerV(rand1V - rand2V);
}

Value Less::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In Less.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return BooleanV(rand1V < rand2V);
}

Value LessEq::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In LessEq.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return BooleanV(rand1V <= rand2V);
}

Value Equal::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In Equal.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return BooleanV(rand1V == rand2V);
}

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In GreaterEq.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return BooleanV(rand1V >= rand2V);
}

Value Greater::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1 -> v_type != V_INT || rand2 -> v_type != V_INT)
        throw RuntimeError("<Evaluation> In Greater.");
    int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
    int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
    return BooleanV(rand1V > rand2V);
}

Value IsEq::evalRator(const Value &rand1, const Value &rand2) {
    int rand1T = rand1 -> v_type;
    int rand2T = rand2 -> v_type;
    if(rand1T == V_INT && rand1T == V_INT) {
        int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
        int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
        return BooleanV(rand1V == rand1V);
    }
    if(rand1T == V_BOOL && rand2T == V_BOOL) {
        bool rand1V = dynamic_cast<Boolean*>(rand1.get()) -> b;
        bool rand2V = dynamic_cast<Boolean*>(rand2.get()) -> b;
        return BooleanV(rand1V == rand2V);
    }
    if(rand1T == V_SYM && rand2T == V_SYM) {
        std::string rand1V = dynamic_cast<Symbol*>(rand1.get()) -> s;
        std::string rand2V = dynamic_cast<Symbol*>(rand2.get()) -> s;
        return BooleanV(rand1V == rand2V);
    }
    return BooleanV(rand1.get() == rand2.get());
}

Value Cons::evalRator(const Value &rand1, const Value &rand2) {
    return PairV(rand1, rand2);
}

Value IsBoolean::evalRator(const Value &rand) {
    return BooleanV(rand -> v_type == V_BOOL);
}

Value IsFixnum::evalRator(const Value &rand) {
    return BooleanV(rand -> v_type == V_INT);
}

Value IsNull::evalRator(const Value &rand) {
    return BooleanV(rand -> v_type == V_NULL);
}

Value IsPair::evalRator(const Value &rand) {
    return BooleanV(rand -> v_type == V_PAIR);
}

Value IsProcedure::evalRator(const Value &rand) {
    int randT = rand -> v_type;
    return BooleanV(randT == V_PROC || randT == V_PRIMITIVE);
}

Value IsSymbol::evalRator(const Value &rand) {
    return BooleanV(rand -> v_type == V_SYM);
}

Value Not::evalRator(const Value &rand) {
    if(rand -> v_type == V_BOOL) 
        if(!(dynamic_cast<Boolean*>(rand.get()) -> b)) return BooleanV(true);
    return BooleanV(false);
}

Value Car::evalRator(const Value &rand) {
    if(rand -> v_type != V_PAIR)
        throw RuntimeError("<Evaluation> In Car.");
    Pair *randV = dynamic_cast<Pair*>(rand.get());
    return randV -> car;
}

Value Cdr::evalRator(const Value &rand) {
    if(rand -> v_type != V_PAIR)
        throw RuntimeError("<Evaluation> In Cdr.");
    Pair *randV = dynamic_cast<Pair*>(rand.get());
    return randV -> cdr;
}