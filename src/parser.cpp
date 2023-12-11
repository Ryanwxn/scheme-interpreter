#ifndef PARSER 
#define PARSER

// parser of myscheme 

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std :: string;
using std :: vector;
using std :: pair;

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Expr Syntax :: parse(Assoc &env) {
    return this -> ptr -> parse(env);
}

Expr Number :: parse(Assoc &env) {
    return Expr(new Fixnum(this -> n));
}

Expr Identifier :: parse(Assoc &env) {
    return Expr(new Var(this -> s));
}

Expr TrueSyntax :: parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax :: parse(Assoc &env) {
    return Expr(new False());
}

Expr List :: parse(Assoc &env) {
    int arguNum = this -> stxs.size();
    if(!arguNum) return Expr(new Begin(std::vector<Expr>()));
    // get the first token
    Expr lead = this -> stxs[0].parse(env);
    int leadType = lead -> e_type;
    if(leadType == E_VAR) {
        std::string funcName = dynamic_cast<Var*>(lead.get()) -> x;
        // the first token is a primitive
        if(primitives.find(funcName) != primitives.end()) {
            int funcType = primitives[funcName];
            if((funcType >= E_MUL && funcType <= E_GT ) || funcType == E_CONS || funcType == E_EQQ) {
                if(arguNum != 3)
                    throw new RuntimeError("RuntimeError");
                Expr lhs = this -> stxs[1].parse(env);
                Expr rhs = this -> stxs[2].parse(env);
                switch(funcType) {
                    case E_MUL : return Expr(new Mult(lhs, rhs));
                    case E_PLUS : return Expr(new Plus(lhs, rhs));
                    case E_MINUS : return Expr(new Minus(lhs, rhs));
                    case E_LT : return Expr(new Less(lhs, rhs));
                    case E_LE : return Expr(new LessEq(lhs, rhs));
                    case E_GT : return Expr(new Greater(lhs, rhs));
                    case E_GE : return Expr(new GreaterEq(lhs, rhs));
                    case E_EQ : return Expr(new Equal(lhs, rhs));
                    case E_CONS : return Expr(new Cons(lhs, rhs));
                    case E_EQQ : return Expr(new IsEq(lhs, rhs));
                }
            }
            if((funcType >= E_BOOLQ && funcType <= E_PROCQ) || (funcType >= E_NOT && funcType <= E_CDR)) {
                if(arguNum != 2)
                    throw new RuntimeError("RuntimeError");
                Expr expr = this -> stxs[1].parse(env);
                switch(funcType) {
                    case E_NOT : return Expr(new Not(expr));
                    case E_CAR : return Expr(new Car(expr));
                    case E_CDR : return Expr(new Cdr(expr));
                    case E_BOOLQ : return Expr(new IsBoolean(expr));
                    case E_INTQ : return Expr(new IsFixnum(expr));
                    case E_NULLQ : return Expr(new IsNull(expr));
                    case E_PAIRQ : return Expr(new IsPair(expr));
                    case E_PROCQ : return Expr(new IsProcedure(expr));
                    case E_SYMBOLQ : return Expr(new IsSymbol(expr));
                }
            }
            if(funcType == E_VOID) {
                if(arguNum != 1)
                    throw new RuntimeError("RuntimeError");
                return Expr(new MakeVoid());
            }
            if(funcType == E_EXIT) {
                if(arguNum != 1)
                    throw new RuntimeError("RuntimeError");
                return Expr(new Exit());
            }
        }
        
    }
}

#endif