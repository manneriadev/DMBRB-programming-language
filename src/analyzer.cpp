/* #include "analyzer.hpp"
#include <iostream>

void analyzer::analyze(Program& program)
{
    push_new_vision();

    for (auto& module : program.modules)
    {
        for(auto& decl: module->decls)
        {
            if(auto* f = dynamic_cast<FunctionDecl*>(decl.get()))
            {
                AnyFunction func;
                func.name = f->name;

                for(auto& a : f->args) func.args.push_back(a.type.type);

                func.return_type = f->return_type;
                declare_function(func);
            }
            else if(auto* s = dynamic_cast<StructDecl*>(decl.get()))
            {
                AnyStruct st;
                st.name = s->name;

                for(auto& a : s->fields)
                {
                    StructField sf;
                    sf.name = a.name;
                    sf.type = a.type.type;
                    st.fields.push_back(sf);
                }            
            }
        }
    }
}*/