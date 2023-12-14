#ifndef PARSER 
#define PARSER

// parser of myscheme 

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
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
                    throw RuntimeError("<Parser> A function with two parameters gets wrong number of parameters.");
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
                    throw RuntimeError("<Parser> A function with one parameter gets wrong number of parameteres.");
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
                    throw RuntimeError("<Parser> A function with no parameter gets parameters.");
                return Expr(new MakeVoid());
            }
            if(funcType == E_EXIT) {
                if(arguNum != 1)
                    throw RuntimeError("<Parser> A function with no parameter gets parameters.");
                return Expr(new Exit());
            }
        }
        Value func = find(funcName, env);
        // the first token is a variable
        if(func.get() != nullptr) {
            if(func -> v_type == V_PROC) {
                std::vector<Expr> rand;
                for(int i = 1; i < arguNum; ++i)
                    rand.push_back(this -> stxs[i] -> parse(env));
                return Expr(new Apply(lead, rand));
            } else throw RuntimeError("<Parser> The first token is illegel.");
        }
        // the first token is a reserved word
        if(reserved_words.find(funcName) != reserved_words.end()) {
            int funcType = reserved_words[funcName];
            switch(funcType) {
                case E_LET : {
                    if(arguNum != 3 || this -> stxs[1] -> s_type != S_LIST)
                        throw RuntimeError("<Parser> In let.");
                    std::vector<std::pair<std::string, Expr>> arguList;
                    Assoc bodyEnv = env;
                    for(auto &syn : dynamic_cast<List*>(this -> stxs[1].get()) -> stxs) {
                        if(syn -> s_type != S_LIST)
                            throw RuntimeError("<Parser> In let.");
                        std::vector<Syntax> argu = dynamic_cast<List*>(syn.get()) -> stxs;
                        if(argu.size() != 2 || argu[0] -> s_type != S_IDEN)
                            throw RuntimeError("<Parser> In let.");
                        std::string var = dynamic_cast<Identifier*>(argu[0].get()) -> s;
                        Expr expr = argu[1] -> parse(env);
                        bodyEnv = extend(var, expr -> eval(env), bodyEnv);
                        arguList.emplace_back(var, expr);
                    }
                    return Expr(new Let(arguList, this -> stxs[2] -> parse(bodyEnv)));
                }
                case E_LETREC : {
                    if(arguNum != 3 || this -> stxs[1] -> s_type != S_LIST)
                        throw RuntimeError("<Parser> In letrec.");
                    std::vector<std::pair<std::string, Expr>> arguList;
                    Assoc virtualEnv = env;
                    for(auto &syn : dynamic_cast<List*>(this -> stxs[1].get()) -> stxs) {
                        if(syn -> s_type != S_LIST)
                            throw RuntimeError("<Parser> In letrec.");
                        std::vector<Syntax> argu = dynamic_cast<List*>(syn.get()) -> stxs;
                        if(argu.size() != 2 || argu[0] -> s_type != S_IDEN)
                            throw RuntimeError("<Parser> In letrec.");
                        std::string var = dynamic_cast<Identifier*>(argu[0].get()) -> s;
                        virtualEnv = extend(var, Value(nullptr), virtualEnv);
                    }
                    Assoc bodyEnv = env;
                    for(auto &syn : dynamic_cast<List*>(this -> stxs[1].get()) -> stxs) {
                        std::vector<Syntax> argu = dynamic_cast<List*>(syn.get()) -> stxs;
                        std::string var = dynamic_cast<Identifier*>(argu[0].get()) -> s;
                        Expr expr = argu[1] -> parse(virtualEnv);
                        bodyEnv = extend(var, expr -> eval(virtualEnv), bodyEnv);
                    }                    
                    return Expr(new Let(arguList, this -> stxs[2] -> parse(bodyEnv)));                    
                }
                case E_LAMBDA : {
                    if(arguNum != 3 || this -> stxs[1] -> s_type != S_LIST)
                        throw RuntimeError("<Parser> In Lambda.");
                    std::vector<std::string> paras;
                    for(auto &syn : dynamic_cast<List*>(this -> stxs[1].get()) -> stxs) {
                        if(syn -> s_type != S_IDEN)
                            throw RuntimeError("<Parser> In Lambda.");
                        paras.push_back(dynamic_cast<Identifier*>(syn.get()) -> s);
                    }
                    return Expr(new Lambda(paras, this -> stxs[2] -> parse(env)));
                }
                case E_IF : {
                    if(arguNum != 4)
                        throw RuntimeError("<Parser> In If.");
                    return Expr(new If(this -> stxs[1] -> parse(env), this -> stxs[2] -> parse(env), this -> stxs[3] -> parse(env)));
                }
                case E_BEGIN : {
                    std::vector<Expr> exprs;
                    for(int i = 1; i < arguNum; ++i)
                        exprs.push_back(this -> stxs[i] -> parse(env));
                    return Expr(new Begin(exprs));
                }
                case E_QUOTE : {
                    if(arguNum != 2)
                        throw RuntimeError("<Parser> In Quote.");
                    return Expr(new Quote(this -> stxs[1]));
                }
            }
        }
    } else {
        if(lead -> e_type == E_PROCQ) {
            std::vector<Expr> paras;
            for(int i = 1; i < arguNum; ++i)
                paras.push_back(this -> stxs[i] -> parse(env));
            return Expr(new Apply(lead, paras));
        } else throw RuntimeError("<Parser> The first token is illegel.");
    }
    throw RuntimeError("<Parser> Can't match any condition.");
    return Expr(nullptr);
}

#endif