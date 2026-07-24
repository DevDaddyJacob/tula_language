# Tula BNF Grammar Syntax

EBNF Syntax Notes:
- `=` definition
- `;` termination
- `|` alternation / "or"
- `[...]` optional
- `{...}` repetition
- `(...)` grouping
- `? ... ?` special sequence
- `"..."` terminal / literal string
- `(* ... *)` comment
- concatenation is inferred by 2 terms next to each other


---

```
(* currently unused, possible for removal once finalized *)
char_white_space = ? white space characters ? ;

(* currently unused, possible for removal once finalized *)
char_inline_white_space = char_white_space - "\n" ;

char_all = ? all visible characters ? ;

char_alpha = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K"
            | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V"
            | "W" | "X" | "Y" | "Z" | "a" | "b" | "c"| "d" | "e" | "f" | "g"
            | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r"
            | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;
            
char_numeric = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

char_alpha_numeric = char_alpha | char_numeric ;

digits = char_numeric {char_numeric} ;


field_separator = "," ;


identifier = (char_alpha | "_") {char_alpha_numeric | "_"} ;

identifier_list = identifier {field_separator identifier} ;


literal_string = '"' {char_all - '"'} '"' ;

literal_char = "'" (char_all - "'" | "\'") "'" ;

literal_true = "true" ;

literal_false = "false" ;

literal_boolean = literal_true | literal_false ;

literal_integer = digits ["b" | "ub" | "s" | "us" | "u" | "l" | "ul"] ;

literal_decimal = digits "." digits ["d"] ;

literal_numeric = literal_integer | literal_decimal ;

literal = literal_string | literal_char | literal_boolean | literal_numeric ;


operator_arithmetic = "+" | "-" | "*" | "^" | "/" | "%" ;
            
operator_comparison_binary = ">" | "<" | "==" | "!=" | ">=" | "<=" | "and"
                            | "or" ;

operator_comparison_unary = "not" ;
            
operator_binary = operator_arithmetic | operator_comparison_binary ;
            
operator_unary = operator_comparison_unary | "++" | "--" ;
            
operator = operator_binary | operator_unary ;


array_definition = "[" [expression {field_separator expression} [field_separator]] "]" ;


table_field = "[" (literal_string | literal_char | digits) "]" "=" expression ;

table_definition = "{" [table_field {field_separator table_field} [field_separator]] "}" ;


member_accessor = "." identifier ;

index_accessor = "[" expression "]" ;

call_accessor = "(" [expression_list] ")" ;

accessor = member_accessor | index_accessor | call_accessor ;


function_body = "(" [identifier_list] ")" block ;

function_definition_base = ("function" | "func") identifier function_body ;

function_definition_local = "def" function_definition_base ;

function_definition_global = "def" "global" function_definition_base ;

function_definition_value = ("function" | "func") function_body ;


expression_is_set = "isSet" "(" identifier ")" ;

expression_pre_increment = "++" identifier ;

expression_post_increment = identifier "++" ;

expression_pre_decrement = "--" identifier ;

expression_post_decrement = identifier "--" ;

(*
    Expression precedence tiers, ordered loosest (top) to tightest (bottom).
    Each binary tier is left-associative and uses {...} repetition, which the
    parser folds left. expression_exponent is the exception: it is written
    right-recursively so right-associativity is enforced by the grammar itself
    rather than left to the parser.
*)
expression = expression_logical_or ;

expression_logical_or = expression_logical_and {"or" expression_logical_and} ;

expression_logical_and = expression_equality {"and" expression_equality} ;

expression_equality = expression_comparison
                        {("==" | "!=") expression_comparison} ;

expression_comparison = expression_additive
                        {(">" | "<" | ">=" | "<=") expression_additive} ;

expression_additive = expression_multiplicative
                        {("+" | "-") expression_multiplicative} ;

expression_multiplicative = expression_exponent
                                {("*" | "/" | "%") expression_exponent} ;

expression_exponent = expression_unary ["^" expression_exponent] ;

expression_unary = operator_comparison_unary expression_unary
                   | expression_pre_increment
                   | expression_pre_decrement
                   | expression_postfix ;

expression_postfix = expression_primary {accessor}
                     | expression_post_increment
                     | expression_post_decrement ;

expression_primary = literal
                     | identifier
                     | expression_is_set
                     | function_definition_value
                     | array_definition
                     | table_definition
                     | "(" expression ")" ;
            
expression_list = expression {field_separator expression} ;
            

variable_base = ("variable" | "var") identifier ;

variable_assignment = variable_base "=" expression ;

variable_local_definition = ("define" | "def") variable_assignment ;

variable_global_definition = ("define" | "def") "global" variable_assignment ;

variable_local_set = "set" variable_assignment ;

variable_global_set = "set" "global" variable_assignment ;

variable_local_unset = "unset" variable_base ;

variable_global_unset = "unset" "global" variable_base ;


constant_assignment = ("constant" | "const") identifier "=" expression ;

constant_local_definition = ("define" | "def") constant_assignment ;

constant_global_definition = ("define" | "def") "global" constant_assignment ;


(*
    A function_call_statement is a postfix expression constrained to end in a
    call_accessor. Requiring the trailing call structurally (rather than by a
    note) means bare identifiers and accessor chains without a call are not
    statements, while reusing expression_primary and accessor keeps it in step
    with the expression tiers and allows chained calls (e.g. f()(), a.b().c()).
*)
function_call_statement = expression_primary {accessor} call_accessor ;


(*
    A condition may be any expression, but it must be of boolean type; there is
    no truthy/falsy coercion. The type rule is enforced during semantic
    analysis, not by the grammar: a non-boolean condition whose type is
    statically known (e.g. if ("foo")) is a compile-time error, otherwise it is
    a runtime error.
*)
condition_resolvable = expression ;


comparison_statement_single = "if" "(" condition_resolvable ")" block ;

comparison_statement = comparison_statement_single
                        {"else" comparison_statement_single} ["else" block] ;


conditional_iteration_while_check =  "while" "(" condition_resolvable ")" ;

conditional_iteration_while_statement = conditional_iteration_while_check block ;

conditional_iteration_do_statement = "do" block conditional_iteration_while_check ;


numeric_iteration_initialization = [variable_local_definition] ;

numeric_iteration_condition = [condition_resolvable] ;

numeric_iteration_update = [ variable_local_set | variable_global_set 
                            | expression_pre_increment
                            | expression_post_increment
                            | expression_pre_decrement
                            | expression_post_decrement
                            ] ;

numeric_iteration_statement = "for" "(" numeric_iteration_initialization
                                field_separator numeric_iteration_condition 
                                field_separator numeric_iteration_update ")"
                                block ;


statement = "break" | "continue" 
            | expression_is_set
            | function_definition_local
            | function_definition_global
            | function_call_statement
            | constant_local_definition
            | constant_global_definition
            | variable_local_definition
            | variable_global_definition
            | variable_local_set
            | variable_global_set
            | variable_local_unset
            | variable_global_unset
            | conditional_iteration_while_statement
            | conditional_iteration_do_statement
            | numeric_iteration_statement
            ;


return_statement = "return" [expression] ;


block = "{" {statement} [return_statement] "}" ;

```