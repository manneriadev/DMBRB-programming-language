#include "ASTPrinter.hpp"

ASTPrinter::ASTPrinter(std::ostream& out)
    : out(out) {}

void ASTPrinter::print_prefix(){
    for(size_t i = 0 ; i < current_space ; ++i){
        out << ' ';
    }

    if(current_value == 2){
        out << "/---";
    }
    else if(current_value == 1){
        out << "\\---";
    }
}

void ASTPrinter::print_node(const std::string& text){
    print_prefix();
    out << text << "\n";
}

void ASTPrinter::push(size_t value){
    current_space += 5;
    current_value = value;
}

std::string ASTPrinter::binary_to_string(binary_oper op){
    switch(op){
        case binary_oper::PLUS: return "+";
        case binary_oper::MINUS: return "-";
        case binary_oper::STAR: return "*";
        case binary_oper::SLASH: return "/";
        case binary_oper::PERCENT: return "%";
        case binary_oper::EQEQ: return "==";
        case binary_oper::NOTEQ: return "!=";
        case binary_oper::GREATER: return ">";
        case binary_oper::LESS: return "<";
        default: return "?";
    }
}

std::string ASTPrinter::unary_to_string(unary_oper op){
    switch(op){
        case unary_oper::NEGATIVE: return "-";
        case unary_oper::POSITIVE: return "+";
        case unary_oper::NOT: return "!";
        case unary_oper::TILDA: return "~";
        default: return "?";
    }
}

std::string ASTPrinter::assign_to_string(assign_type op){
    switch(op){
        case assign_type::EQ: return "=";
        case assign_type::PLUSEQ: return "+=";
        case assign_type::MINUSEQ: return "-=";
        default: return "?=";
    }
}

std::string ASTPrinter::type_to_string(const Type& type){
    switch(type.base){
        case InnerType::INT32: return "int32";
        case InnerType::INT64: return "int64";
        case InnerType::FLOAT32: return "float32";
        case InnerType::FLOAT64: return "float64";
        case InnerType::BOOL: return "bool";
        case InnerType::CHAR: return "char";
        case InnerType::VOID: return "void";
        case InnerType::STRUCT: return type.struct_name;
        default: return "unknown";
    }
}

void ASTPrinter::print(Program& program){
    out << "PROGRAM\n";

    for(auto& item : program.items){
        current_space = 0;
        current_value = 0;

        if(auto* module = dynamic_cast<Module*>(item.get())){
            print_node("MODULE[" + module->name + "]");

            current_space += 5;

            for(auto& decl : module->decls){
                current_value = 1;
                decl->accept(*this);
            }

            current_space -= 5;
        }
        else {
            current_value = 1;
            item->accept(*this);
        }
    }
}

void ASTPrinter::visit(LiteralExpr&){
    print_node("{Literal}");
}

void ASTPrinter::visit(VariableExpr& node){
    print_node("{Variable}" + node.name);
}

void ASTPrinter::visit(BinaryExpr& node){
    print_node("{Binary}[" + binary_to_string(node.op) + "]");
    
    if(node.left){
        push(2);
        node.left->accept(*this);
        current_space -= 5;
    }


    if(node.right){
        push(1);
        node.right->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(UnaryExpr& node){
    print_node("{Unary}[" + unary_to_string(node.op) + "]");

    if(node.expr){
        push(1);
        node.expr->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(AssignmentExpr& node){
    print_node("{Assignment}[" + assign_to_string(node.type) + "]");

    if(node.target){
        push(2);
        node.target->accept(*this);
        current_space -= 5;
    }

    if(node.value){
        push(1);
        node.value->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(CallExpr& node){
    print_node("{Call}" + node.name);

    for(auto& arg : node.args){
        push(1);
        arg->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(MemberAccessExpr& node){
    print_node("{MemberAccess}" + node.field);

    if(node.object){
        push(1);
        node.object->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(IndexExpr& node){
    print_node("{Index}");

    if(node.object){
        push(2);
        node.object->accept(*this);
        current_space -= 5;
    }

    if(node.index){
        push(1);
        node.index->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(TernaryExpr& node){
    print_node("{Ternary}");

    push(2);
    node.cond->accept(*this);
    current_space -= 5;

    push(2);
    node.thenExpr->accept(*this);
    current_space -= 5;

    push(1);
    node.elseExpr->accept(*this);
    current_space -= 5;
}

void ASTPrinter::visit(CastExpr& node){
    print_node("{Cast}[" + type_to_string(node.target_type) + "]");

    push(1);
    node.expr->accept(*this);
    current_space -= 5;
}

void ASTPrinter::visit(ArrayLiteralExpr& node){
    print_node("{ArrayLiteral}");

    for(auto& el : node.elements){
        push(1);
        el->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(EmptyExpr&){
    print_node("{EmptyExpr}");
}

void ASTPrinter::visit(BlockStmt& node){
    print_node("{Block}");

    current_space += 5;

    for(auto& item : node.items){
        current_value = 1;
        item->accept(*this);
    }

    current_space -= 5;
}

void ASTPrinter::visit(ExprStmt& node){
    print_node("{ExprStmt}");

    push(1);
    node.expr->accept(*this);
    current_space -= 5;
}

void ASTPrinter::visit(IfStmt& node){
    print_node("{IfStmt}");

    push(2);
    node.cond->accept(*this);
    current_space -= 5;

    push(1);
    node.then_block->accept(*this);
    current_space -= 5;

    if(node.else_block){
        push(1);
        node.else_block->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(WhenStmt&){
    print_node("{WhenStmt}");
}

void ASTPrinter::visit(WhileStmt& node){
    print_node("{WhileStmt}");

    push(2);
    node.cond->accept(*this);
    current_space -= 5;

    push(1);
    node.body->accept(*this);
    current_space -= 5;
}

void ASTPrinter::visit(ForStmt& node){
    print_node("{ForStmt}" + node.var_name);
}

void ASTPrinter::visit(ReturnStmt& node){
    print_node("{ReturnStmt}");

    if(node.value){
        push(1);
        node.value->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(BreakStmt&){
    print_node("{BreakStmt}");
}

void ASTPrinter::visit(ContinueStmt&){
    print_node("{ContinueStmt}");
}

void ASTPrinter::visit(VarDecl& node){
    std::string text = "{VarDecl}" + node.name;

    if(node.has_type){
        text += ":" + type_to_string(node.type.type);
    }

    print_node(text);

    if(node.init){
        push(1);
        node.init->accept(*this);
        current_space -= 5;
    }
}

void ASTPrinter::visit(FunctionDecl& node){
    print_node("{Function}" + node.name);

    current_space += 5;

    for(auto& arg : node.args){
        current_value = 1;

        print_node(
            "{Arg}" + arg.name +
            ":" + type_to_string(arg.type.type)
        );
    }

    if(node.body){
        node.body->accept(*this);
    }

    current_space -= 5;
}

void ASTPrinter::visit(StructDecl& node){
    print_node("{Struct}" + node.name);

    current_space += 5;

    for(auto& field : node.fields){
        current_value = 1;

        print_node(
            "{Field}" + field.name +
            ":" + type_to_string(field.type.type)
        );
    }

    current_space -= 5;
}