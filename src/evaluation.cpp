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

#include <thread>
// #include <atomic>
// #include <vector>

// Multiple threads
// a function for multiple threads
// void _multiEval(Expr& expr, Value& val, Assoc& env, std::atomic_bool& flag) {
//     try{
//         val = expr -> eval(env);
//     } catch (const RuntimeError& RE) {
//         // Throw error by setting the variable flag.
//         flag = true;
//     }
// }

// Evaluate a bunch of expressions in multiple threads.
void multiEval(std::vector<Expr>& exprs, std::vector<Value>& vals, Assoc& env) {
    std::atomic_bool flag = false;
    int num = exprs.size();
    vals.clear();
    for(int i = 0; i < num; ++i) vals.push_back(exprs[i] -> eval(env));
    

    /*
    for(int i = 0; i < num; ++i)
        vals[i] = exprs[i] -> eval(env);
    */
}

Value Let::eval(Assoc &env) {
    Assoc bodyEnv = env;
    std::vector<Expr> exprs;
    std::vector<Value> vals;
    int paraNum = this -> bind.size();
    for(int i = 0; i < paraNum; ++i)
        exprs.push_back(this -> bind[i].second);
    multiEval(exprs, vals, env);
    for(int i = 0; i < paraNum; ++i)
        bodyEnv = extend(this -> bind[i].first, vals[i], bodyEnv);
    return this -> body -> eval(bodyEnv);
}

Value Lambda::eval(Assoc &env) {
    std::vector<std::string> paras;
    Assoc bodyEnv = env;
    for(auto &para : this -> x)
        paras.push_back(para);
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
    std::vector<Expr> exprs;
    std::vector<Value> vals;
    for(int i = 0; i < paraNum; ++i)
        exprs.push_back(this -> rand[i]);
    multiEval(exprs, vals, e);
    for(int i = 0; i < paraNum; ++i)
        bodyEnv = extend(clos -> parameters[i], vals[i], bodyEnv);
    return clos -> e -> eval(bodyEnv);
}

Value Letrec::eval(Assoc &env) {
    Assoc virtualEnv = env;
    for(auto &[argName, argValue] : this -> bind)
        virtualEnv = extend(argName, Value(nullptr), virtualEnv);
    Assoc bodyEnv = virtualEnv;
    int paraNum = this -> bind.size();
    std::vector<Expr> exprs;
    std::vector<Value> vals;
    for(int i = 0; i < paraNum; ++i)
        exprs.push_back(this -> bind[i].second);
    multiEval(exprs, vals, virtualEnv);
    for(int i = 0; i < paraNum; ++i) 
        bodyEnv = extend(this -> bind[i].first, vals[i], bodyEnv);
    exprs.clear(), vals.clear();
    for(int i = 0; i < paraNum; ++i)
        exprs.push_back(this -> bind[i].second);
    multiEval(exprs, vals, bodyEnv);
    for(int i = 0; i < paraNum; ++i)
        modify(this -> bind[i].first, vals[i], bodyEnv);
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
    std::vector<Expr> exprs;
    std::vector<Value> vals;
    exprs.push_back(this -> rand1);
    exprs.push_back(this -> rand2);
    multiEval(exprs, vals, e);
    return this -> evalRator(vals[0], vals[1]);
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
    ValueType rand1T = rand1 -> v_type;
    ValueType rand2T = rand2 -> v_type;
    if(rand1T == V_INT && rand1T == V_INT) {
        int rand1V = dynamic_cast<Integer*>(rand1.get()) -> n;
        int rand2V = dynamic_cast<Integer*>(rand2.get()) -> n;
        return BooleanV(rand1V == rand2V);
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
    if(rand -> v_type != V_PAIR) {
        throw RuntimeError("<Evaluation> In Car.");
    }
    Pair *randV = dynamic_cast<Pair*>(rand.get());
    return randV -> car;
}

Value Cdr::evalRator(const Value &rand) {
    if(rand -> v_type != V_PAIR)
        throw RuntimeError("<Evaluation> In Cdr.");
    Pair *randV = dynamic_cast<Pair*>(rand.get());
    return randV -> cdr;
}

Value primitivePlus() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Plus(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveMinus() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Minus(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveMult() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Mult(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveLess() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Less(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveLessEq() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new LessEq(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveEqual() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Equal(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveGreaterEq() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new GreaterEq(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveGreater() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Greater(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveCons() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new Cons(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveIsEq() {
  return ClosureV(
    std::vector<std::string> {"operand1", "operand2"},
    Expr(new IsEq(Expr(new Var("operand1")), Expr(new Var("operand2")))),
    empty()
  );
}

Value primitiveExit() {
  return ClosureV(
    std::vector<std::string>(),
    Expr(new Exit()),
    empty()
  );
}

Value primitiveMakeVoid() {
  return ClosureV(
    std::vector<std::string>(),
    Expr(new MakeVoid()),
    empty()
  );
}

Value primitiveCar() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new Car(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveCdr() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new Cdr(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveNot() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new Not(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveIsFixnum() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new IsFixnum(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveIsBoolean() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new IsBoolean(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveIsSymbol() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new IsSymbol(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveIsNull() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new IsNull(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveIsPair() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new IsPair(Expr(new Var("operand")))),
    empty()
  );
}

Value primitiveIsProcedure() {
  return ClosureV(
    std::vector<std::string> {"operand"},
    Expr(new IsProcedure(Expr(new Var("operand")))),
    empty()
  );
}