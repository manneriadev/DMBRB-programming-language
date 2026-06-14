//codegen.cpp

#include "codegen.hpp"
#include <stdexcept>

Codegen::Codegen(std::ostream& out) : out(out) {}

void Codegen::write(const std::string& s) 
{ 
    out << s; 
}

void Codegen::writeln(const std::string& s)
{
    for(int i = 0; i < indent; ++i) out << "    ";
    out << s << "\n";
}

void Codegen::begin_indent()
{
    ++indent;
}
void Codegen::end_indent()
{
    --indent;
}

std::string Codegen::mangle(const std::string& module, const std::string& name)
{
    if(module.empty()) return name;
    return module + "__" + name;
}

std::string Codegen::decl_to_c(const Type& t, const std::string& name)
{
    if(t.is_array && !t.arrays.empty())
    {
        Type elem = t;
        elem.is_array = false;
        elem.arrays.clear();
        return type_to_c(elem) + " " + name + "[" + std::to_string(t.arrays[0].size_val) + "]";
    }
    return type_to_c(t) + " " + name;
}

std::string Codegen::type_to_c(const Type& t)
{
    std::string base;
    switch(t.base)
    {
        case InnerType::INT8: base = "int8_t"; break;
        case InnerType::INT16: base = "int16_t"; break;
        case InnerType::INT32: base = "int32_t"; break;
        case InnerType::INT64: base = "int64_t"; break;
        case InnerType::UINT8: base = "uint8_t"; break;
        case InnerType::UINT16: base = "uint16_t"; break;
        case InnerType::UINT32: base = "uint32_t"; break;
        case InnerType::UINT64: base = "uint64_t"; break;
        case InnerType::FLOAT32: base = "float"; break;
        case InnerType::FLOAT64: base = "double"; break;
        case InnerType::BOOL: base = "int"; break;
        case InnerType::CHAR: base = "char"; break;
        case InnerType::VOID: base = "void"; break;
        case InnerType::STRUCT: base = t.struct_name; break;
        default: base = "void"; break;
    }
    if(t.is_pointer) base += "*";
    return base;
}

void Codegen::generate(Program& program)
{
    writeln("#include <stdint.h>");
    writeln("#include <stdio.h>");
    writeln("#include <stdlib.h>");
    writeln("#include <math.h>");
    writeln("#include <string.h>");
    writeln("#include <unistd.h>");
    writeln("");
    program.accept(*this);
}

// expressions

void Codegen::visit(LiteralExpr& node)
{
    if(std::holds_alternative<int64_t>(node.value)) out << std::get<int64_t>(node.value);
    else if(std::holds_alternative<double>(node.value)) out << std::get<double>(node.value);
    else if(std::holds_alternative<bool>(node.value)) out << (std::get<bool>(node.value) ? "1" : "0");
    else if(std::holds_alternative<char>(node.value)) out << "'" << std::get<char>(node.value) << "'";
    else if(std::holds_alternative<std::string>(node.value)) out << "\"" << std::get<std::string>(node.value) << "\"";
    else if(std::holds_alternative<uint64_t>(node.value)) out << std::get<uint64_t>(node.value);
}

void Codegen::visit(VariableExpr& node)
{
    out << node.name;
}

void Codegen::visit(SelfExpr&)
{
    out << "self";
}

void Codegen::visit(AddrOfExpr& node)
{
    out << "(&";
    node.ref->accept(*this);
    out << ")";
}

void Codegen::visit(DerefExpr& node)
{
    out << "(*";
    node.ref->accept(*this);
    out << ")";
}

void Codegen::visit(BinaryExpr& node)
{
    std::string op;
    switch(node.op)
    {
        case binary_oper::PLUS: op = "+"; break;
        case binary_oper::MINUS: op = "-"; break;
        case binary_oper::STAR: op = "*"; break;
        case binary_oper::STARSTAR: op = ""; break;
        case binary_oper::SLASH: op = "/"; break;
        case binary_oper::SLASHSLASH: op = "/"; break;
        case binary_oper::PERCENT: op = "%"; break;
        case binary_oper::LESSLESS: op = "<<"; break;
        case binary_oper::GREATERGREATER: op = ">>"; break;
        case binary_oper::XOR: op = "^"; break;
        case binary_oper::PIPE: op = "|"; break;
        case binary_oper::AMP: op = "&"; break;
        case binary_oper::EQEQ: op = "=="; break;
        case binary_oper::NOTEQ: op = "!="; break;
        case binary_oper::GREATER: op = ">"; break;
        case binary_oper::LESS: op = "<"; break;
        case binary_oper::GREATEREQ: op = ">="; break;
        case binary_oper::LESSEQ: op = "<="; break;
        case binary_oper::AMPERAMPER: op = "&&"; break;
        case binary_oper::PIPEPIPE: op = "||"; break;
    }

    if(node.op == binary_oper::STARSTAR)
    {
        out << "pow(";
        node.left->accept(*this);
        out << ", ";
        node.right->accept(*this);
        out << ")";
        return;
    }

    out << "(";
    node.left->accept(*this);
    out << " " << op << " ";
    node.right->accept(*this);
    out << ")";
}

void Codegen::visit(UnaryExpr& node)
{
    std::string op;
    switch(node.op)
    {
        case unary_oper::NEGATIVE: op = "-"; break;
        case unary_oper::POSITIVE: op = "+"; break;
        case unary_oper::NOT: op = "!"; break;
        case unary_oper::TILDA: op = "~"; break;
    }
    out << "(" << op;
    node.expr->accept(*this);
    out << ")";
}

void Codegen::visit(AssignmentExpr& node)
{
    std::string op;
    switch(node.type)
    {
        case assign_type::EQ: op = "="; break;
        case assign_type::PLUSEQ: op = "+="; break;
        case assign_type::MINUSEQ: op = "-="; break;
        case assign_type::STAREQ: op = "*="; break;
        case assign_type::SLASHEQ: op = "/="; break;
        case assign_type::PERCENTEQ: op = "%="; break;
        case assign_type::AMPEQ: op = "&="; break;
        case assign_type::PIPEEQ: op = "|="; break;
    }
    if(dynamic_cast<EmptyExpr*>(node.value.get())) return;

    node.target->accept(*this);
    out << " " << op << " ";
    node.value->accept(*this);
}

void Codegen::visit(CallExpr& node)
{
    if(node.name == "print") // builtins
    {
        out << "printf(";
        for(size_t i = 0; i < node.args.size(); ++i)
        {
            if(i > 0) out << ", ";
            node.args[i]->accept(*this);
        }
        out << ")";
        return;
    }
    
    if(node.name == "exit")
    {
        out << "exit(";
        if(!node.args.empty()) node.args[0]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "sin")
    {
        out << "sinf(";
        node.args[0]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "cos")
    {
        out << "cosf(";
        node.args[0]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "memset")
    {
        out << "memset(";
        node.args[0]->accept(*this);
        out << ", ";
        node.args[1]->accept(*this);
        out << ", ";
        node.args[2]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "putchar")
    {
        out << "putchar(";
        node.args[0]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "usleep")
    {
        out << "usleep(";
        node.args[0]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "malloc")
    {   
        out << "malloc(";
        node.args[0]->accept(*this);
        out << ")";
        return;
    }

    if(node.name == "free")
    {
        out << "free(";
        node.args[0]->accept(*this);
        out << ")";
        return;
    }

    std::string name = node.resolved_name.empty() ? node.name : node.resolved_name;
    out << name << "(";
    for(size_t i = 0; i < node.args.size(); ++i)
    {
        if(i > 0) out << ", ";
        node.args[i]->accept(*this);
    }
    out << ")";
}

void Codegen::visit(MethodCallExpr& node)
{
    if(node.is_struct_method)
    {
        std::string fname = node.resolved_name.empty()
            ? (node.struct_name + "__" + node.method)
            : node.resolved_name;

        out << fname << "(&";
        node.object->accept(*this);  // self
        for(auto& arg : node.args)
        {
            out << ", ";
            arg->accept(*this);
        }
        out << ")";
        return;
    }

    std::string fname = node.resolved_name;
    if(fname.empty())
    {
        std::string mod_name;
        Expr* obj = node.object.get();
        while(auto* mem = dynamic_cast<MemberAccessExpr*>(obj))
        {
            mod_name = mem->field + (mod_name.empty() ? "" : "__" + mod_name);
            obj = mem->object.get();
        }
        if(auto* var = dynamic_cast<VariableExpr*>(obj)) mod_name = var->name + (mod_name.empty() ? "" : "__" + mod_name);
        fname = mod_name + "__" + node.method;
    }

    out << fname << "(";
    for(size_t i = 0; i < node.args.size(); ++i)
    {
        if(i > 0) out << ", ";
        node.args[i]->accept(*this);
    }
    out << ")";
}

void Codegen::visit(MemberAccessExpr& node)
{
    if(dynamic_cast<SelfExpr*>(node.object.get()))
    {
        node.object->accept(*this);
        out << "->" << node.field;
        return;
    }
    node.object->accept(*this);
    out << "." << node.field;
}

void Codegen::visit(IndexExpr& node)
{
    node.object->accept(*this);
    out << "[";
    node.index->accept(*this);
    out << "]";
}

void Codegen::visit(TernaryExpr& node)
{
    out << "(";
    node.cond->accept(*this);
    out << " ? ";
    node.thenExpr->accept(*this);
    out << " : ";
    node.elseExpr->accept(*this);
    out << ")";
}

void Codegen::visit(CastExpr& node)
{
    out << "((" << type_to_c(node.target_type) << ")(";
    node.expr->accept(*this);
    out << "))";
}

void Codegen::visit(ArrayLiteralExpr& node)
{
    out << "{";
    for(size_t i = 0; i < node.elements.size(); ++i)
    {
        if(i > 0) out << ", ";
        node.elements[i]->accept(*this);
    }
    out << "}";
}

void Codegen::visit(EmptyExpr&)
{
    out << "NULL";
}

// statements

void Codegen::visit(BlockStmt& node)
{
    for(auto& item : node.items)
    {
        for(int i = 0; i < indent; ++i) out << "    ";
        item->accept(*this);
        if(dynamic_cast<ExprStmt*>(item.get()) || dynamic_cast<VarDecl*>(item.get())) out << ";\n";
        else if(!dynamic_cast<IfStmt*>(item.get()) &&
                !dynamic_cast<WhileStmt*>(item.get()) &&
                !dynamic_cast<ForStmt*>(item.get()) &&
                !dynamic_cast<WhenStmt*>(item.get()))
            out << "\n";
    }
}

void Codegen::visit(ExprStmt& node)
{
    node.expr->accept(*this);
}

void Codegen::visit(IfStmt& node)
{
    out << "if (";
    node.cond->accept(*this);
    out << ") {\n";
    begin_indent();
    node.then_block->accept(*this);
    end_indent();
    for(int i = 0; i < indent; ++i) out << "    ";
    out << "}";
    if(node.else_block)
    {
        out << " else {\n";
        begin_indent();
        node.else_block->accept(*this);
        end_indent();
        for(int i = 0; i < indent; ++i) out << "    ";
        out << "}";
    }
    out << "\n";
}

void Codegen::visit(WhenStmt& node) // chain of if else (чем гуще лес if else if else)
{
    bool first = true;
    for(auto& [cond, body] : node.cases)
    {
        if(!first) for(int i = 0; i < indent; ++i) out << "    ";
        if(first) { out << "if ("; first = false; }
        else out << "else if (";
        cond->accept(*this);
        out << ") {\n";
        begin_indent();
        body->accept(*this);
        end_indent();
        for(int i = 0; i < indent; ++i) out << "    ";
        out << "}\n";
    }
    if(node.else_block)
    {
        for(int i = 0; i < indent; ++i) out << "    ";
        out << "else {\n";
        begin_indent();
        node.else_block->accept(*this);
        end_indent();
        for(int i = 0; i < indent; ++i) out << "    ";
        out << "}\n";
    }
}

void Codegen::visit(WhileStmt& node)
{
    out << "while (";
    node.cond->accept(*this);
    out << ") {\n";
    begin_indent();
    node.body->accept(*this);
    end_indent();
    for(int i = 0; i < indent; ++i) out << "    ";
    out << "}\n";
}

void Codegen::visit(ForStmt& node)
{
    // for i = start:end:step -> for(int i = start; i < end; i += step)
    out << "for (int64_t " << node.var_name << " = ";
    node.start->accept(*this);
    out << "; " << node.var_name << " < ";
    node.end->accept(*this);
    out << "; " << node.var_name;
    if(node.step)
    {
        out << " += ";
        node.step->accept(*this);
    }
    else
    {
        out << "++";
    }
    out << ") {\n";
    begin_indent();
    node.body->accept(*this);
    end_indent();
    for(int i = 0; i < indent; ++i) out << "    ";
    out << "}\n";
}

void Codegen::visit(ReturnStmt& node)
{
    out << "return";
    if(node.value)
    {
        out << " ";
        node.value->accept(*this);
    }
    out << ";\n";
}

void Codegen::visit(BreakStmt&) { out << "break;\n"; }
void Codegen::visit(ContinueStmt&) { out << "continue;\n"; }

// declarations

void Codegen::visit(VarDecl& node)
{
    if(node.type.is_const) out << "const ";
    out << decl_to_c(node.type.type, node.name);
    if(node.init)
    {
        if(dynamic_cast<EmptyExpr*>(node.init.get()))
        {
            if(node.type.type.is_pointer)
                out << " = NULL";
        }
        else
        {
            out << " = ";
            node.init->accept(*this);
        }
    }
}

void Codegen::visit(FunctionDecl& node)
{
    std::string local_name = node.mangled_name.empty() ? node.name : node.mangled_name;
    std::string fname = mangle(current_module, local_name);
    std::string rtype = type_to_c(node.return_type);

    out << rtype << " " << fname << "(";
    for(size_t i = 0; i < node.args.size(); ++i)
    {
        if(i > 0) out << ", ";
        out << decl_to_c(node.args[i].type.type, node.args[i].name);
    }
    out << ") {\n";
    begin_indent();
    if(node.body) node.body->accept(*this);
    end_indent();
    out << "}\n\n";
}

void Codegen::visit(StructDecl& node)
{
    std::string sname = current_module.empty() ? node.name : mangle(current_module, node.name);
    out << "typedef struct {\n";
    for(auto& f : node.fields)
    {
        out << "    " << decl_to_c(f.type.type, f.name) << ";\n";
    }
    out << "} " << sname << ";\n\n";

    std::string prev = current_module;
    current_module = sname;
    for(auto& method : node.decls)
    {
        std::string mname = method->mangled_name.empty() ? (sname + "__" + method->name) : method->mangled_name;
        out << type_to_c(method->return_type) << " ";
        out << mname << "(" << sname << "* self";
        for(auto& a : method->args) out << ", " << decl_to_c(a.type.type, a.name);
        out << ") {\n";
        begin_indent();
        if(method->body) method->body->accept(*this);
        end_indent();
        out << "}\n\n";
    }
    current_module = prev;
}

void Codegen::visit(ImportDecl&) {} 

void Codegen::visit(Module& node)
{
    std::string prev = current_module;
    current_module = node.name;

    for(auto& sub : node.submodules)
    {
        current_module = node.name + "__" + sub->name;
        sub->accept(*this);
        current_module = node.name;
    }

    for(auto& decl : node.decls) 
    {
        decl->accept(*this);
        if(dynamic_cast<VarDecl*>(decl.get())) out << ";\n";
    }
    
    current_module = prev;
}

void Codegen::visit(Program& node)
{
    for(auto& item : node.items)
    {
        for(int i = 0; i < indent; ++i) out << "    ";
        item->accept(*this);
        if(dynamic_cast<VarDecl*>(item.get()) || dynamic_cast<ExprStmt*>(item.get()))
        {
            out << ";\n";
        }
    }
}