/*
    Спецификация к языку программирования - Dmitriibor'ba (dmbrb)

    --- Общее описание ---

    Разрабатываемый язык программирования - компилируемый и статически типизированный язык.
    Синтаксис вдохновлён Julia, архитектура и устройство - Rust/C.

    Язык ориентирован на:
    - простой и читаемый синтаксис
    - статическую типизацию с выводом типов
    - value semantics (все значения передаются по значению)
    - минимальную, но строгую модель выполнения
    - единообразие синтаксиса — все блоки открываются через begin и закрываются через end

    --- Лексика ---

    Ключевые слова:
    function, end, begin, if, else, while, for, when, return, struct, module,
    var, const, true, false, none, break, continue

    Идентификаторы:
    последовательности букв, цифр и _, не начинающиеся с цифры

    Литералы:
    - целые числа:    123
    - вещественные:   1.23
    - строки:         "hello"
    - символы:        'a'
    - булевы:         true, false
    - отсутствие:     none

    Комментарии:
    однострочные: # комментарий

    --- Грамматика ---

program: (module | decl)*
module: 'module' IDENTIFIER 'begin' (module | decl)* 'end'
decl: var_decl | function_decl | struct_decl
var_decl: 'var' 'const'? IDENTIFIER (':' type)?|('=' expr)?
function_decl: 'function' IDENTIFIER '(' pair_arg (',' pair_arg)* / () ')'  '::' type 'begin' block 'end'
struct_decl: 'struct' IDENTIFIER 'begin' pair_arg* 'end'
pair_arg: IDENTIFIER ':' 'const'? type
type: primitive_type '?'? | '[' type ';' expr? ']' '?'? | 'struct' IDENTIFIER '?'?
primitive_type: 'int8' | 'int16' | 'int32' | 'int64' | 'int' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float32' | 'float64' | 'float' | 'bool' | 'char' | 'void'
block: (decl | stmt)*
stmt: if_stmt | when_stmt | while_stmt | for_stmt | return_stmt | break_stmt | continue_stmt | expr_stmt
if_stmt: 'if' expr 'begin' block ('else' 'begin' block)? 'end'
when_stmt: 'when' (expr '->' 'begin' block 'end')* ('else' '->' 'begin' block 'end')? 'end'
while_stmt: 'while' expr 'begin' block 'end'
for_stmt: 'for' IDENTIFIER '=' expr ':' expr (':' expr)? 'begin' block 'end'
return_stmt: 'return' expr?
break_stmt: 'break'
continue_stmt: 'continue'
expr_stmt: expr
expr: binary_expr
binary_expr: unary_expr (('=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=') | ternary_expr | binary_op binary_expr)*
binary_op: '||' | '&&' | '|' | '^' | '&' | '==' | '!=' | '<' | '>' | '<=' | '>=' | '<<' | '>>' | '+' | '-' | '*' | '/' | '%' | '//' | '**'
ternary_expr: expr '?' expr ':' expr
unary_expr: ('+' | '-' | '!' | '~') unary_expr | postfix_expr
postfix_expr: primary_expr postfix*
postfix: '.' IDENTIFIER | '[' expr ']' | '(' (expr (',' expr)*)? ')' | '::' type
primary_expr: INT_LITERAL | FLOAT_LITERAL | STRING_LITERAL | CHAR_LITERAL | 'true' | 'false' | 'none' | IDENTIFIER | '(' expr ')' | '[' (expr (';' expr)*)? ']'

    --- Синтаксис ---

    Общая структура программы:

    программа состоит из набора объявлений верхнего уровня и модулей.
    точка входа — функция main:

    function main()::int32 begin
        return 0
    end

    --- Блоки кода ---

    все блоки открываются через begin и закрываются через end:

    if x > 0 begin
        print(x)
    end

    function foo()::void begin
        ...
    end

    struct Point begin
        x: int32
        y: int32
    end

    module math begin
        ...
    end

    --- Переменные ---

    переменные мутабельны по умолчанию.

    var a: int32 = 5
    var b = 10          # вывод типа

    иммутабельные переменные объявляются через const:

    var const c: int32 = 15

    константа не может быть переназначена после инициализации.

    --- Функции ---

    function add(a: int32, b: int32)::int32 begin
        return a + b
    end

    особенности:
    - типы параметров обязательны
    - тип возвращаемого значения задаётся через ::
    - если тип не указан — функция возвращает void
    - return обязателен для не-void функций

    --- Управление потоком ---

    if x > 0 begin
        print("positive")
    else begin
        print("non-positive")
    end

    when
        x > 0 -> begin
            print("positive")
        end
        x < 0 -> begin
            print("negative")
        end
        else -> begin
            print("zero")
        end
    end

    while x > 0 begin
        x = x - 1
    end

    for i = 0:10 begin
        print(i)
    end

    for i = 0:10:2 begin    # с шагом
        print(i)
    end

    поддерживаются break и continue внутри циклов.

    --- Структуры ---

    struct Point begin
        x: int32
        y: int32
    end

    создание через массив-литерал:

    var p: struct Point = [1, 2]

    доступ к полям:

    p.x

    структуры — чистые типы данных, методов не содержат.

    --- Модули ---

    module math begin
        function add(a: int32, b: int32)::int32 begin
            return a + b
        end

        module utils begin
            function clamp(x: int32, lo: int32, hi: int32)::int32 begin
                return x
            end
        end
    end

    доступ к содержимому модуля через точку:

    math.add(1, 2)
    math.utils.clamp(x, 0, 100)
    math.PI

    особенности:
    - модули могут быть вложенными
    - имена переменных не могут совпадать с именами модулей
    - внутри модуля допускаются: функции, структуры, переменные, вложенные модули

    --- Массивы ---

    var a: [int32; 3] = [1, 2, 3]

    индексация начинается с 0:

    a[0]

    выход за границы приводит к runtime error.

    динамический массив:

    var b: [int32;] = [1, 2, 3]

    --- Система типов ---

    встроенные типы:

    int8, int16, int32, int64     # знаковые целые
    uint8, uint16, uint32, uint64 # беззнаковые целые
    int                           # псевдоним int32
    float32, float64              # вещественные
    float                         # псевдоним float32
    bool                          # булев
    char                          # символ
    void                          # отсутствие значения (только для функций)

    --- Вывод типов ---

    var x = 5       # int64
    var y = 3.14    # float64

    --- Приведение типов ---

    x::int32        # постфиксный оператор ::

    допустимые приведения:
    - int -> float
    - float -> int (с потерей точности)
    - bool -> int

    --- void ---

    void — тип, обозначающий отсутствие возвращаемого значения.
    используется только как тип функций.
    переменные типа void запрещены.

    --- none и опциональные типы ---

    none — специальное значение, обозначающее отсутствие значения.
    допустимо только для опциональных типов T?:

    var x: int32? = none
    var y: float32? = 3.14

    var z = none    # ошибка компиляции — невозможно вывести тип

    правила:
    - T? совместим с T и none
    - T не совместим с none
    - none допустим только для T?

    --- Операторы ---

    арифметические:   + - * / % ** //
    сравнения:        == != < > <= >=
    логические:       && || !
    побитовые:        & | ^ ~ << >>
    присваивание:     = += -= *= /= %= &= |=
    тернарный:        cond ? expr1 : expr2
    приведение типа:  expr::type

    особенности:
    - ** — возведение в степень
    - // — целочисленное деление

    --- Приоритет операторов (от высокого к низкому) ---

    1.  () [] . ::    (группировка, индексация, доступ, каст — постфикс)
    2.  ! ~ -         (унарные)
    3.  **
    4.  * / % //
    5.  + -
    6.  << >>
    7.  < > <= >=
    8.  == !=
    9.  &
    10. ^
    11. |
    12. &&
    13. ||
    14. ? :
    15. = += -= *= /= %= &= |=

    --- Область видимости ---

    используется лексическая область видимости.
    переменная видна внутри блока в котором объявлена.
    допускается shadowing.
    использование до объявления запрещено.
    имена переменных не могут совпадать с именами модулей.

    --- Семантика значений ---

    все значения передаются по значению.
    при присваивании создаётся независимая копия.
    аргументы функций передаются по значению.

    --- Ошибки выполнения ---

    runtime error: <message> at line <N>

    обрабатываются:
    - деление на ноль
    - выход за границы массива
    - некорректное приведение типов

    --- Переполнение ---

    переполнение целых чисел происходит по модулю (wrap-around).

    --- Встроенные функции ---

    print(x)    # вывод значения
    input()     # ввод строки
    exit(code)  # завершение программы
*/