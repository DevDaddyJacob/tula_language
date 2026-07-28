# Tula BNF Grammar Syntax

EBNF Syntax Notes:
- `=` definition
- `;` termination
- `|` alternation / "or"
- `[...]` optional
- `{...}` repetition
- `(...)` grouping
- `? ... ?` special sequence
- `"..."` & `'...'` terminal / literal string (the single quote is used
  exclusively for representing a double-quote char)
- `(* ... *)` comment
- concatenation is inferred by 2 terms next to each other


---

```
char_white_space = ? white space characters ? ;

char_all = ? all visible characters ? ;

char_escaped = "\" char_all ;

char_any = char_white_space | char_all | char_escaped ;

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


literal_string = '"' {char_any - ('"' | ? newline ?)} '"' ;

literal_char = "'" (char_any - ("'" | ? newline ?)) "'" ;

literal_true = "true" ;

literal_false = "false" ;

literal_boolean = literal_true | literal_false ;

literal_integer_signed = ["-"] digits ["b" | "s" | "l"] ;

literal_integer_unsigned = digits ("ub" | "us" | "u" | "ul") ;

literal_integer = literal_integer_signed | literal_integer_unsigned ;

literal_decimal = ["-"] [digits] "." digits ["d"] ;

literal_numeric = literal_integer | literal_decimal ;

literal = literal_string | literal_char | literal_boolean | literal_numeric ;


operator_equality = "==" | "!=" ;

operator_comparison = ">" | "<" | ">=" | "<=" ;

operator_additive = "+" | "-" ;

operator_multiplicative = "*" | "/" | "%" ;


array_definition = "[" [expression {field_separator expression} [field_separator]] "]" ;


(*
    A table key is any expression (matching index_accessor's "[" expression "]"
    form) but must resolve to a string, char, or integer type. As with
    conditions, the type rule is enforced during semantic analysis, not by the
    grammar: statically when the key type is known, otherwise at runtime.
*)
table_field = "[" expression "]" "=" expression ;

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


expression_identifier = identifier {member_accessor | index_accessor} ;

expression_is_set = "isSet" "(" expression_identifier ")" ;

expression_pre_increment = "++" expression_identifier ;

expression_post_increment = expression_identifier "++" ;

expression_pre_decrement = "--" expression_identifier ;

expression_post_decrement = expression_identifier "--" ;

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
                        {operator_equality expression_comparison} ;

expression_comparison = expression_additive
                        {operator_comparison expression_additive} ;

expression_additive = expression_multiplicative
                        {operator_additive expression_multiplicative} ;

expression_multiplicative = expression_exponent
                            {operator_multiplicative expression_exponent} ;

expression_exponent = expression_unary ["^" expression_exponent] ;

expression_unary = ("not" expression_unary)
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
            

(*
    Assignment is always keyword-gated: "=" only appears inside a
    variable_assignment or constant_assignment, always introduced by def/define
    (definition) or set (reassignment). There is no bare "x = expr" statement -
    to assign you write, e.g., "def var x = expr" or "set var x = expr".
*)
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


return_statement = "return" [expression] ;


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
            | comparison_statement
            | return_statement
            ;


block = "{" {statement} "}" ;


program = {statement} ;

```