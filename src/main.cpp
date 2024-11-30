#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include "RE.hpp"
#include <sstream>
#include <iostream>
#include <map>


extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

void initGloabalEnv(Assoc& global) {
    global = extend("+", primitivePlus(), global);
    global = extend("-", primitiveMinus(), global);
    global = extend("*", primitiveMult(), global);
    global = extend("<", primitiveLess(), global);
    global = extend("<=", primitiveLessEq(), global);
    global = extend("=", primitiveEqual(), global);
    global = extend(">=", primitiveGreaterEq(), global);
    global = extend(">", primitiveGreater(), global);
    global = extend("eq?", primitiveIsEq(), global);
    global = extend("cons", primitiveCons(), global);
    global = extend("void", primitiveMakeVoid(), global);
    global = extend("exit", primitiveExit(), global);
    global = extend("not", primitiveNot(), global);
    global = extend("car", primitiveCar(), global);
    global = extend("cdr", primitiveCdr(), global);
    global = extend("fixnum?", primitiveIsFixnum(), global);
    global = extend("boolean?", primitiveIsBoolean(), global);
    global = extend("null?", primitiveIsNull(), global);
    global = extend("pair?", primitiveIsPair(), global);
    global = extend("symbol?", primitiveIsSymbol(), global);
    global = extend("procedure?", primitiveIsProcedure(), global);
}

void REPL()
{
    // read - evaluation - print loop
    Assoc global_env = empty();
    initGloabalEnv(global_env);
    while (1)
    {   
        #ifndef ONLINEJUDGE
            std::cout << "scm> ";
        #endif
        Syntax stx = readSyntax(std :: cin); // read
        try
        {
            Expr expr = stx -> parse(global_env); // parse
            // stx -> show(std :: cout); // syntax print
            Value val = expr -> eval(global_env);
            if (val -> v_type == V_TERMINATE)
                break;
            val -> show(std :: cout); // value print
        }
        catch (const RuntimeError &RE)
        {
            // std :: cout << RE.message();
            std :: cout << "RuntimeError";
        }
        puts("");
    }
}


int main(int argc, char *argv[]) {
    initPrimitives();
    initReservedWords();
    REPL();
    return 0;
}
