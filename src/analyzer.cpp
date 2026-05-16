//analyzer.cpp
#include "analyzer.hpp"

#include <iostream>

void analyzer::push_new_vision()
{
    visions.emplace_back();
}
void analyzer::pop_last_vision()
{
    if(!visions.empty()) visions.pop_back();
}

bool analyzer::declare_variable(VarDecl& node)
{
    if(is_module_name(node.name))
    {
        error("variable name conflicts with module: " + node.name, &node);
        return false;
    }

    auto& current = visions.back().variables;

    if(current.find(node.name) != current.end())
    {
        error("double declaration of variable: " + node.name, &node);
        return false;
    }

    Type final_type;
    bool has_type = node.has_type;
    bool has_init = node.has_init;

    if(has_type) final_type = node.type.type;

    if(has_init)
    {
        Type init_type = visit_expr(*node.init);
        if(has_type)
        {
            if(!can_convert(init_type, final_type))
            {
                error("type mismatch in initializer for variable: " + node.name, &node);
                return false;
            }
        }
        else
        {
            final_type = init_type;
        }
    }

    if(!has_type && !has_init)
    {
        error("variable must have type or initializer: " + node.name, &node);
        return false;
    }

    current[node.name] = AnyVariable{.name = node.name, .type = final_type};

    if(!node.has_type)
    {
        node.type.type = final_type;
        node.has_type = true;
    }
    return true;
}

bool analyzer::declare_function(FunctionDecl& node)
{
    auto& current = visions.back().functions;

    if(current.find(node.name) != current.end())
    {
        error("double declaration of function: " + node.name, &node);
        return false;
    }

    AnyFunction func;
    func.name = node.name;
    func.return_type = node.return_type;

    for(const auto& a : node.args) func.args.push_back(a.type.type);

    current[node.name] = func;
    return true;
}

bool analyzer::declare_struct(StructDecl& node)
{
    auto& current = visions.back().structs;

    if(current.find(node.name) != current.end())
    {
        error("double declaration of struct: " + node.name, &node);
        return false;
    }

    AnyStruct st;
    st.name = node.name;

    for(const auto& f : node.fields)
    {
        StructField field;

        field.name = f.name;
        field.type = f.type.type;

        st.fields.push_back(field);
    }

    current[node.name] = st;
    return true;
}

AnyVariable* analyzer::resolve_variable(const std::string& name)
{
    for(auto it = visions.rbegin(); it != visions.rend(); ++it)
    {
        auto found = it->variables.find(name);
        if(found != it->variables.end()) return &found->second;
    }

    return nullptr;
}

AnyFunction* analyzer::resolve_function(const std::string& name)
{
    for(auto it = visions.rbegin(); it != visions.rend(); ++it)
    {
        auto found = it->functions.find(name);
        if(found != it->functions.end())return &found->second;
    }

    return nullptr;
}

AnyStruct* analyzer::resolve_struct(const std::string& name)
{
    for(auto it = visions.rbegin(); it != visions.rend(); ++it)
    {
        auto found = it->structs.find(name);
        if(found != it->structs.end()) return &found->second;
    }

    return nullptr;
}

AnyModule* analyzer::resolve_module(const std::string& name)
{
    auto it = global_modules.find(name);
    if(it != global_modules.end()) return &it->second;
    return nullptr;
}

AnyModule* analyzer::resolve_module_chain(Expr* expr)
{
    if(auto* var = dynamic_cast<VariableExpr*>(expr))
        return resolve_module(var->name);

    if(auto* mem = dynamic_cast<MemberAccessExpr*>(expr))
    {
        AnyModule* parent = resolve_module_chain(mem->object.get());
        if(!parent) return nullptr;
        auto it = parent->modules.find(mem->field); // module search in module
        if(it == parent->modules.end()) return nullptr;
        return &it->second;
    }

    return nullptr;
}

bool analyzer::is_module_name(const std::string& name)
{
    return global_modules.find(name) != global_modules.end();
}

bool analyzer::is_lvalue(Expr& expr)
{
    if(dynamic_cast<VariableExpr*>(&expr))
    {
        if(auto* var = dynamic_cast<VariableExpr*>(&expr))
        {
            auto v = resolve_variable(var->name);
            if(v && v->is_const)
            {
                return false;
            }
        }
        return true;
    }
    if(dynamic_cast<MemberAccessExpr*>(&expr)) return true;
    if(dynamic_cast<IndexExpr*>(&expr)) return true;
    return false;
}

bool analyzer::is_none_type(const Type& t)
{
    return t.base == InnerType::VOID && t.is_optional;
}

bool analyzer::is_integer(const Type& type) const
{
    switch(type.base)
    {
        case InnerType::INT8:
        case InnerType::INT16:
        case InnerType::INT32:
        case InnerType::INT64:
        case InnerType::UINT8:
        case InnerType::UINT16:
        case InnerType::UINT32:
        case InnerType::UINT64:
            return true;
        default:
            return false;
    }
}

bool analyzer::is_float(const Type& type) const
{
    return type.base == InnerType::FLOAT32 || type.base == InnerType::FLOAT64;
}

bool analyzer::is_numeric(const Type& type) const
{
    return is_integer(type) || is_float(type);
}

bool analyzer::is_bool(const Type& type) const
{
    return type.base == InnerType::BOOL;
}

bool analyzer::is_string(const Type& type) const
{
    return type.base == InnerType::CHAR && !type.arrays.empty();
}

Type analyzer::visit_expr(Expr& node)
{
    node.accept(*this);
    return current_expr_type;
}

void analyzer::error(const std::string& message, const Node* node)
{

    if(failed) return;
    failed = true;

    if(node)
        std::fprintf(stderr, "analyzer error at %zu:%zu: %s\n", node->line, node->column, message.c_str());
    else
        std::fprintf(stderr, "analyzer error: %s\n", message.c_str());
}

bool analyzer::type_equal(const Type& a, const Type& b) const
{
    return a.struct_name == b.struct_name && a.base == b.base && a.arrays.size() == b.arrays.size() && a.is_optional == b.is_optional;
}

bool analyzer::can_convert(const Type& from, const Type& to) const
{
    return (is_integer(from) && is_float(to) || is_float(from) && is_float(to) || is_integer(from) && is_integer(to) || type_equal(from, to)) ? true : false;
}

bool analyzer::is_comparable(const Type& a, const Type& b) const
{
    return (type_equal(a,b) || is_numeric(a) && is_numeric(b) || is_string(a) && is_string(b) || is_bool(a) && is_bool(b)) ? true : false;
}

void analyzer::analyze(Program& program)
{
    failed = false;
    push_new_vision();
    program.accept(*this);
    pop_last_vision();
}

// expressions

void analyzer::visit(LiteralExpr& node)
{
    Type type{};

    if(std::holds_alternative<int64_t>(node.value))
    {
        type.base = InnerType::INT64;
    }
    else if(std::holds_alternative<double>(node.value))
    {
        type.base = InnerType::FLOAT64;
    }
    else if(std::holds_alternative<bool>(node.value))
    {
        type.base = InnerType::BOOL;
    }
    else if(std::holds_alternative<char>(node.value))
    {
        type.base = InnerType::CHAR;
    }
    else if(std::holds_alternative<std::string>(node.value))
    {
        type.base = InnerType::CHAR;
        type.is_array = true;

        ArrayDim d;
        d.is_dynamic = true;
        type.arrays.push_back(d);
    }

    current_expr_type = type;
}

void analyzer::visit(VariableExpr& node)
{
    auto v = resolve_variable(node.name);

    if(!v)
    {
        error("undefined variable: " + node.name, &node);
        return;
    }

    current_expr_type = v->type;
}

void analyzer::visit(BinaryExpr& node) 
{
    Type left = visit_expr(*node.left);
    Type right = visit_expr(*node.right);

    if(is_none_type(left) || is_none_type(right))
    {
        error("cannot use NONE in binary expression", &node);
        return;
    }

    if(!can_convert(left, right) && !can_convert(right, left))
    {
        error("type mismatch in binary expression", &node);
        return;
    }

    switch(node.op)
    {
        case binary_oper::PLUS:
            if(is_numeric(left) && is_numeric(right))
            {
                current_expr_type = (is_float(left) || is_float(right)) ? Type{InnerType::FLOAT64} : Type{InnerType::INT64};
                return;
            }

            if(is_string(left) && is_string(right))
            {
                current_expr_type = left;
                return;
            }

            error("invalid '+' operands", &node);
            return;
        case binary_oper::MINUS:
        case binary_oper::STAR:
        case binary_oper::SLASH:
        case binary_oper::PERCENT:
        case binary_oper::STARSTAR:

            if(is_numeric(left) && is_numeric(right))
            {
                current_expr_type = (is_float(left) || is_float(right)) ? Type{InnerType::FLOAT64} : Type{InnerType::INT64};
                return;
            }

            error("arithmetic operator requires numeric types", &node);
            return;

        case binary_oper::EQEQ:
        case binary_oper::NOTEQ:
        case binary_oper::LESS:
        case binary_oper::GREATER:
        case binary_oper::LESSEQ:
        case binary_oper::GREATEREQ:

            if(is_comparable(left, right))
            {
                current_expr_type = Type{InnerType::BOOL};
                return;
            }

            error("invalid comparison types", &node);
            return;

        case binary_oper::AMPERAMPER:
        case binary_oper::PIPEPIPE:

            if(!is_bool(left) || !is_bool(right))
            {
                error("logical operators require bool operands" , &node);
                return;
            }

            current_expr_type.base = InnerType::BOOL;
            return;

        case binary_oper::AMP:
        case binary_oper::PIPE:
        case binary_oper::XOR:

            if(is_integer(left) && is_integer(right))
            {
                current_expr_type = left;
                return;
            }

            error("bit operators require integers", &node);
            return;

        case binary_oper::LESSLESS:
        case binary_oper::GREATERGREATER:

            if(is_integer(left) && is_integer(right))
            {
                current_expr_type = left;
                return;
            }

            error("shift operators require integers", &node);
            return;

        default:
            error("unknown binary operator" , &node);
            return;
    }
}

void analyzer::visit(UnaryExpr& node)
{
     Type t = visit_expr(*node.expr);

    if(is_none_type(t))
    {
        error("cannot apply unary operator to NONE", &node);
        return;
    }

    switch(node.op)
    {
        case unary_oper::NEGATIVE:
        case unary_oper::POSITIVE:
        {
            if(!is_numeric(t))
            {
                error("unary +/- requires numeric type", &node);
                return;
            }

            current_expr_type = t;
            return;
        }

        case unary_oper::TILDA:
        {
            if(!is_integer(t))
            {
                error("bit NOT requires integer type", &node);
                return;
            }

            current_expr_type = t;
            return;
        }

        case unary_oper::NOT:
        {
            if(!is_bool(t))
            {
                error("logical NOT requires bool type", &node);
                return;
            }

            current_expr_type = Type{InnerType::BOOL};
            return;
        }
    }
}

void analyzer::visit(AssignmentExpr& node) 
{
    if(!node.target) return;

    if(!is_lvalue(*node.target))
    {
        error("left side of assignment is not assignable", &node);
        return;
    }
    
    Type left = visit_expr(*node.target);
    Type right = visit_expr(*node.value);

    if(is_none_type(right))
    {
        if(!left.is_optional)
        {
            error("cannot assign 'none' to non-optional type", &node);
            return;
        }

        current_expr_type = left;
        return;
    }

    if(is_none_type(left))
    {
        error("cannot assign to 'none'", &node);
        return;
    }

    if(!can_convert(right, left))
    {
        error("assignment type mismatch", &node);
        return;
    }

    current_expr_type = left;

    
}

void analyzer::visit(CallExpr& node)
{
    auto func = resolve_function(node.name);

    if(!func)
    {
        error("undefined function: " + node.name, &node);
        return;
    }

    if(func->args.size() != node.args.size())
    {
        error("argument count mismatch in call: " + node.name, &node);
        return;
    }

    for(size_t i = 0; i < node.args.size(); i++)
    {
        Type arg_t = visit_expr(*node.args[i]);

        if(!can_convert(arg_t, func->args[i]))
        {
            error("argument type mismatch in call: " + node.name, &node);
            return;
        }
    }

    current_expr_type = func->return_type;
}

void analyzer::visit(MemberAccessExpr& node)
{
    AnyModule* mod = resolve_module_chain(node.object.get());
    if(mod)
    {
        auto it = mod->variables.find(node.field);
        if(it != mod->variables.end())
        {
            current_expr_type = it->second.type;
            return;
        }

        if(mod->modules.find(node.field) != mod->modules.end()) // example: var x = math.utils
        {
            error("cannot use module as value: " + node.field, &node);
            return;
        }

        error("unknown variable in module: " + node.field, &node);
        return;
    }

    // structs field

    Type obj = visit_expr(*node.object);
    if(obj.base != InnerType::STRUCT || obj.struct_name.empty())
    {
        error("member access on non-struct type", &node);
        return;
    }
    auto st = resolve_struct(obj.struct_name);
    if(!st) { error("unknown struct type: " + obj.struct_name, &node); return; }
    for(const auto& f : st->fields)
    {
        if(f.name == node.field) { current_expr_type = f.type; return; }
    }
    error("unknown field: " + node.field, &node);
}

void analyzer::visit(MethodCallExpr& node)
{
    AnyModule* mod = resolve_module_chain(node.object.get());
    if(!mod)
    {
        error("cannot resolve method call target", &node);
        return;
    }

    auto it = mod->functions.find(node.method);
    if(it == mod->functions.end())
    {
        error("unknown function in module: " + node.method, &node);
        return;
    }

    AnyFunction& func = it->second;
    if(func.args.size() != node.args.size())
    {
        error("argument count mismatch: " + node.method, &node);
        return;
    }

    for(size_t i = 0; i < node.args.size(); i++)
    {
        Type arg_t = visit_expr(*node.args[i]);
        if(!can_convert(arg_t, func.args[i]))
        {
            error("argument type mismatch: " + node.method, &node);
            return;
        }
    }

    current_expr_type = func.return_type;
}

void analyzer::visit(IndexExpr& node)
{
    Type obj = visit_expr(*node.object);
    Type idx = visit_expr(*node.index);

    if(!is_integer(idx))
    {
        error("index must be integer", &node);
        return;
    }

    if(obj.arrays.empty())
    {
        error("indexing non-array type", &node);
        return;
    }

    obj.arrays.pop_back();
    obj.is_array = !obj.arrays.empty();

    current_expr_type = obj;
}

void analyzer::visit(TernaryExpr& node)
{
    Type cond = visit_expr(*node.cond);

    if(!is_bool(cond))
    {
        error("ternary condition must be bool", &node);
        return;
    }

    Type a = visit_expr(*node.thenExpr);
    Type b = visit_expr(*node.elseExpr);

    if(!can_convert(a, b) && !can_convert(b, a))
    {
        error("ternary branch type mismatch", &node);
        return;
    }

    current_expr_type = a; // left
}

void analyzer::visit(CastExpr& node)
{
    Type from = visit_expr(*node.expr);

    if(!can_convert(from, node.target_type) && !can_convert(node.target_type, from))
    {
        error("invalid cast", &node);
        return;
    }

    current_expr_type = node.target_type;
}

void analyzer::visit(ArrayLiteralExpr& node)
{
    if(node.elements.empty())
    {
        error("empty array literal", &node);
        return;
    }

    Type first = visit_expr(*node.elements[0]);

    for(size_t i = 1; i < node.elements.size(); i++)
    {
        Type t = visit_expr(*node.elements[i]);

        if(!can_convert(t, first))
        {
            error("array element type mismatch", &node);
            return;
        }
    }

    Type result = first;
    result.is_array = true;

    ArrayDim d;
    d.is_dynamic = true;
    result.arrays.push_back(d);

    current_expr_type = result;
}

void analyzer::visit(EmptyExpr& node)
{
    Type t;
    t.base = InnerType::VOID;
    t.is_optional = true;

    current_expr_type = t;
}

// statements 

void analyzer::visit(BlockStmt& node)
{
    push_new_vision();

    for(auto& item : node.items)
        item->accept(*this);

    pop_last_vision();
}

void analyzer::visit(ExprStmt& node)
{
    visit_expr(*node.expr);
}

void analyzer::visit(IfStmt& node)
{
    Type cond = visit_expr(*node.cond);

    if(!is_bool(cond))
    {
        error("if condition must be bool", &node);
        return;
    }

    node.then_block->accept(*this);
    if(node.else_block) node.else_block->accept(*this);
}

void analyzer::visit(WhenStmt& node)
{
    for(auto& [cond, block] : node.cases)
    {
        Type c = visit_expr(*cond);

        if(!is_bool(c))
        {
            error("when condition must be bool", &node);
            return;
        }

        block->accept(*this);
    }

    if(node.else_block) node.else_block->accept(*this);
}

void analyzer::visit(WhileStmt& node)
{
    Type cond = visit_expr(*node.cond);

    if(!is_bool(cond))
    {
        error("while condition must be bool", &node);
        return;
    }

    loop_depth++;
    node.body->accept(*this);
    loop_depth--;
}

void analyzer::visit(ForStmt& node)
{
    Type start = visit_expr(*node.start);
    Type end = visit_expr(*node.end);

    if(!is_numeric(start) || !is_numeric(end))
    {
        error("for range must be numeric", &node);
        return;
    }

    if(node.step)
    {
        Type step = visit_expr(*node.step);
        if(!is_numeric(step))
        {
            error("for step must be numeric", &node);
            return;
        }
    }

    push_new_vision();

    AnyVariable iter;
    iter.name = node.var_name;
    iter.type = (is_float(start) || is_float(end)) ? Type{InnerType::FLOAT64} : Type{InnerType::INT64};
    visions.back().variables[iter.name] = iter;

    ++loop_depth;
    node.body->accept(*this);
    --loop_depth;

    pop_last_vision();
}

void analyzer::visit(ReturnStmt& node)
{
    if(!current_function)
    {
        error("'return' used outside of function", &node);
        return;
    }

    const Type& expected = current_function->return_type;

    if(!node.value)
    {
        if(expected.base != InnerType::VOID) error("missing return value in non-void function: " + current_function->name, &node);
        return;
    }

    Type actual = visit_expr(*node.value);
    
    if(!can_convert(actual, expected)) error("return type mismatch in function: " + current_function->name, &node);
}

void analyzer::visit(BreakStmt& node)
{
    if(loop_depth == 0) error("'break' used outside of loop", &node);
}

void analyzer::visit(ContinueStmt& node)
{
    if(loop_depth == 0) error("'continue' used outside of loop", &node);
}

// declarations 

void analyzer::visit(VarDecl& node)
{
    declare_variable(node);
}

void analyzer::visit(FunctionDecl& node)
{
    declare_function(node);
    push_new_vision();

    for(const auto& a : node.args)
    {
        AnyVariable v;
        v.name = a.name;
        v.type = a.type.type;
        visions.back().variables[v.name] = v;
    }

    auto* prev_function = current_function;
    current_function = &node;

    if(node.body) node.body->accept(*this);

    current_function = prev_function;
    pop_last_vision();
}

void analyzer::visit(StructDecl& node)
{
    declare_struct(node);
}

void analyzer::visit(Module& node)
{
    AnyModule mod;
    mod.name = node.name;

    for(auto& decl : node.decls)
    {
        if(auto* f = dynamic_cast<FunctionDecl*>(decl.get()))
        {
            AnyFunction func;
            func.name = f->name;
            func.return_type = f->return_type;
            for(auto& a : f->args) func.args.push_back(a.type.type);
            mod.functions[f->name] = func;
        }
        else if(auto* s = dynamic_cast<StructDecl*>(decl.get()))
        {
            AnyStruct st;
            st.name = s->name;
            for(auto& f : s->fields)
            {
                StructField field;
                field.name = f.name;
                field.type = f.type.type;
                st.fields.push_back(field);
            }
            mod.structs[s->name] = st;
        }
        else if(auto* v = dynamic_cast<VarDecl*>(decl.get()))
        {
            AnyVariable var;
            var.name = v->name;
            if(v->has_type) var.type = v->type.type;
            mod.variables[v->name] = var;
        }
    }

    for(auto& sub : node.submodules)
    {
        visit(*sub);
        auto it = global_modules.find(sub->name);
        if(it != global_modules.end())
            mod.modules[sub->name] = it->second;
    }

    global_modules[node.name] = mod;

    push_new_vision();
    for(auto& decl : node.decls) decl->accept(*this);
    pop_last_vision();
}

void analyzer::visit(Program& node)
{
    for(auto& item : node.items) item->accept(*this);
}