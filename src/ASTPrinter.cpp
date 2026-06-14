#include "ASTPrinter.hpp"

ASTPrinter::ASTPrinter(std::ostream& out) : out(out) {}

void ASTPrinter::print_prefix()
{
    for(size_t i = 0; i < current_space; ++i) out << ' ';

    if(current_value == 2) out << "/---";
    else if(current_value == 1) out << "\\---";
}

void ASTPrinter::print_node(const std::string& text)
{
    print_prefix();
    out << text << "\n";
}

void ASTPrinter::push(size_t value)
{
    current_space += 5;
    current_value = value;
}

std::string ASTPrinter::binary_to_string(binary_oper op)
{
    switch(op)
    {
        case binary_oper::PLUS:           return "+";
        case binary_oper::MINUS:          return "-";
        case binary_oper::STAR:           return "*";
        case binary_oper::STARSTAR:       return "**";
        case binary_oper::SLASH:          return "/";
        case binary_oper::SLASHSLASH:     return "//";
        case binary_oper::PERCENT:        return "%";
        case binary_oper::LESSLESS:       return "<<";
        case binary_oper::GREATERGREATER: return ">>";
        case binary_oper::XOR:            return "^";
        case binary_oper::PIPE:           return "|";
        case binary_oper::AMP:            return "&";
        case binary_oper::EQEQ:           return "==";
        case binary_oper::NOTEQ:          return "!=";
        case binary_oper::GREATER:        return ">";
        case binary_oper::LESS:           return "<";
        case binary_oper::GREATEREQ:      return ">=";
        case binary_oper::LESSEQ:         return "<=";
        case binary_oper::AMPERAMPER:     return "&&";
        case binary_oper::PIPEPIPE:       return "||";
        default:                          return "?";
    }
}

std::string ASTPrinter::unary_to_string(unary_oper op)
{
    switch(op)
    {
        case unary_oper::NEGATIVE: return "-";
        case unary_oper::POSITIVE: return "+";
        case unary_oper::NOT:      return "!";
        case unary_oper::TILDA:    return "~";
        default:                   return "?";
    }
}

std::string ASTPrinter::assign_to_string(assign_type op)
{
    switch(op)
    {
        case assign_type::EQ:        return "=";
        case assign_type::PLUSEQ:    return "+=";
        case assign_type::MINUSEQ:   return "-=";
        case assign_type::STAREQ:    return "*=";
        case assign_type::SLASHEQ:   return "/=";
        case assign_type::PERCENTEQ: return "%=";
        case assign_type::AMPEQ:     return "&=";
        case assign_type::PIPEEQ:    return "|=";
        default:                     return "?=";
    }
}

std::string ASTPrinter::type_to_string(const Type& type)
{
    std::string base;
    switch(type.base)
    {
        case InnerType::INT8:    base = "int8";    break;
        case InnerType::INT16:   base = "int16";   break;
        case InnerType::INT32:   base = "int32";   break;
        case InnerType::INT64:   base = "int64";   break;
        case InnerType::UINT8:   base = "uint8";   break;
        case InnerType::UINT16:  base = "uint16";  break;
        case InnerType::UINT32:  base = "uint32";  break;
        case InnerType::UINT64:  base = "uint64";  break;
        case InnerType::FLOAT32: base = "float32"; break;
        case InnerType::FLOAT64: base = "float64"; break;
        case InnerType::BOOL:    base = "bool";    break;
        case InnerType::CHAR:    base = "char";    break;
        case InnerType::VOID:    base = "void";    break;
        case InnerType::STRUCT:  base = "struct " + type.struct_name; break;
        default:                 base = "unknown"; break;
    }

    for(auto& dim : type.arrays)
        base = "[" + base + (dim.is_dynamic ? ";]" : ";N]");

    if(type.is_optional) base += "?";
    if(type.is_pointer) base += "*";
    return base;
}

void ASTPrinter::visit(AddrOfExpr& node)
{
    print_node("AddrOf");
    if(node.ref)
    {
        push(1);
        node.ref->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(DerefExpr& node)
{
    print_node("Deref");
    if(node.ref)
    {
        push(1);
        node.ref->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(LiteralExpr&)
{
    print_node("Literal");
}

void ASTPrinter::visit(VariableExpr& node)
{
    print_node("Var: " + node.name);
}

void ASTPrinter::visit(SelfExpr&)
{
    print_node("Self");
}

void ASTPrinter::visit(BinaryExpr& node)
{
    print_node("Binary: " + binary_to_string(node.op));
    if(node.left)  { push(2); node.left->accept(*this);  current_space -= 5; }
    if(node.right) { push(1); node.right->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(UnaryExpr& node)
{
    print_node("Unary: " + unary_to_string(node.op));
    if(node.expr) { push(1); node.expr->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(AssignmentExpr& node)
{
    print_node("Assign: " + assign_to_string(node.type));
    if(node.target) { push(2); node.target->accept(*this); current_space -= 5; }
    if(node.value)  { push(1); node.value->accept(*this);  current_space -= 5; }
}

void ASTPrinter::visit(CallExpr& node)
{
    print_node("Call: " + node.name);
    for(auto& arg : node.args) { push(1); arg->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(MethodCallExpr& node)
{
    print_node("MethodCall: " + node.method);
    if(node.object) { push(2); node.object->accept(*this); current_space -= 5; }
    for(auto& arg : node.args) { push(1); arg->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(MemberAccessExpr& node)
{
    print_node("Member: " + node.field);
    if(node.object) { push(1); node.object->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(IndexExpr& node)
{
    print_node("Index");
    if(node.object) { push(2); node.object->accept(*this); current_space -= 5; }
    if(node.index)  { push(1); node.index->accept(*this);  current_space -= 5; }
}

void ASTPrinter::visit(TernaryExpr& node)
{
    print_node("Ternary");
    push(2); node.cond->accept(*this);     current_space -= 5;
    push(2); node.thenExpr->accept(*this); current_space -= 5;
    push(1); node.elseExpr->accept(*this); current_space -= 5;
}

void ASTPrinter::visit(CastExpr& node)
{
    print_node("Cast: " + type_to_string(node.target_type));
    if(node.expr) { push(1); node.expr->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(ArrayLiteralExpr& node)
{
    print_node("ArrayLiteral");
    for(auto& el : node.elements) { push(1); el->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(EmptyExpr&)
{
    print_node("None");
}

void ASTPrinter::visit(BlockStmt& node)
{
    print_node("Block");
    current_space += 5;
    for(auto& item : node.items)
    {
        current_value = 1;
        item->accept(*this);
    }
    current_space -= 5;
}

void ASTPrinter::visit(ExprStmt& node)
{
    if(node.expr)
    {
        push(1);
        node.expr->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(IfStmt& node)
{
    print_node("If");
    push(2); node.cond->accept(*this); current_space -= 5;
    push(node.else_block ? 2 : 1); node.then_block->accept(*this); current_space -= 5;
    if(node.else_block) { push(1); node.else_block->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(WhenStmt& node)
{
    print_node("When");
    current_space += 5;
    for(auto& [cond, body] : node.cases)
    {
        current_value = 2;
        cond->accept(*this);
        current_value = 1;
        body->accept(*this);
    }
    if(node.else_block) { current_value = 1; node.else_block->accept(*this); }
    current_space -= 5;
}

void ASTPrinter::visit(WhileStmt& node)
{
    print_node("While");
    push(2); node.cond->accept(*this); current_space -= 5;
    push(1); node.body->accept(*this); current_space -= 5;
}

void ASTPrinter::visit(ForStmt& node)
{
    print_node("For: " + node.var_name);
    if(node.start) { push(2); node.start->accept(*this); current_space -= 5; }
    if(node.end)   { push(node.step ? 2 : 1); node.end->accept(*this); current_space -= 5; }
    if(node.step)  { push(1); node.step->accept(*this);  current_space -= 5; }
    if(node.body)  { push(1); node.body->accept(*this);  current_space -= 5; }
}

void ASTPrinter::visit(ReturnStmt& node)
{
    print_node("Return");
    if(node.value) { push(1); node.value->accept(*this); current_space -= 5; }
}

void ASTPrinter::visit(BreakStmt&)    { print_node("Break"); }
void ASTPrinter::visit(ContinueStmt&) { print_node("Continue"); }

void ASTPrinter::visit(VarDecl& node)
{
    std::string text = "Var: " + node.name;
    if(node.has_type) text += " : " + type_to_string(node.type.type);
    if(node.type.is_const) text += " [const]";
    print_node(text);
    if(node.init)
    {
        push(1);
        node.init->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(FunctionDecl& node)
{
    std::string text = "Function: " + node.name + "() :: " + type_to_string(node.return_type);
    print_node(text);
    current_space += 5;
    for(auto& arg : node.args)
    {
        current_value = 1;
        print_node("Arg: " + arg.name + " : " + type_to_string(arg.type.type));
    }
    if(node.body)
    {
        current_value = 1;
        node.body->accept(*this);
    }
    current_space -= 5;
}

void ASTPrinter::visit(StructDecl& node)
{
    print_node("Struct: " + node.name);
    current_space += 5;
    for(auto& field : node.fields)
    {
        current_value = 1;
        print_node("Field: " + field.name + " : " + type_to_string(field.type.type));
    }
    for(auto& decl : node.decls)
    {
        current_value = 1;
        print_node("Functions: " + decl->name + " :: " + type_to_string(decl->return_type));
    }
    current_space -= 5;
}

void ASTPrinter::visit(ImportDecl& node)
{
    std::string text = "Import: " + node.path;
    if(!node.alias.empty()) text += " as " + node.alias;
    print_node(text);
}

void ASTPrinter::visit(Module& node)
{
    print_node("Module: " + node.name);
    current_space += 5;
    for(auto& sub : node.submodules)  { current_value = 1; sub->accept(*this); }
    for(auto& decl : node.decls)     { current_value = 1; decl->accept(*this); }
    current_space -= 5;
}

void ASTPrinter::visit(Program& node)
{
    out << "Program\n";
    current_space = 0;
    for(auto& item : node.items) { current_value = 1; item->accept(*this); }
}

void ASTPrinter::print(Program& program)
{
    visit(program);
}