# Tula BNF Grammar Syntax

```
char_white_space = ? white space characters ? ;

char_inline_white_space = char_white_space - "\n" ;

char_all = ? all visible characters ? ;

char_alpha = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K"
            | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V"
            | "W" | "X" | "Y" | "Z" | "a" | "b" | "c"| "d" | "e" | "f" | "g"
            | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r"
            | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;
            
char_numeric = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

char_alpha_numeric = char_alpha | char_numeric ;

digit = char_numeric {char_numeric} ;


field_separator = "," ;


identifier = char_alpha | "_" {char_alpha_numeric | "_"} ;

identifier_list = identifier {field_separator identifier} ;


literal_string = '"' {char_all - '"'} '"' ;

literal_char = "'" (char_all - "'" | "\'") "'" ;

literal_true = "true" ;

literal_false = "false" ;

literal_boolean = literal_true | literal_false ;

literal_integer = digit ["b" | "ub" | "s" | "us" | "u" | "l" | "ul"] ;

literal_decimal = digit "." digit {digit} ["d"] ;

literal_numeric = literal_integer | literal_decimal ;

literal = literal_string | literal_char | literal_boolean | literal_numeric ;
           
            
operator_binary = "+" | "-" | "*" | "^" | "/" | "%" | ">" | "<" | "=="
                    | "!=" | ">=" | "<=" | "and" | "or" ;
            
operator_uniary = "not" | "++" | "--" ;
            
operator = operator_binary | operator_uniary ;

operator_arithmetic = "+" | "-" | "*" | "^" | "/" | "%" ;
            
operator_comparison_binary = ">" | "<" | "==" | "!=" | ">=" | "<=" | "and"
                            | "or" ;

operator_comparison_uniary = "not" ;


array_definition = "[" [expression {field_separator expression} [field_separator]] "]" ;

array_accessor = "[" digit "]" ;


table_field = "[" (literal_string | literal_char | digit) "]" "=" expression ;

table_definition = "{" [table_field {field_separator table_field} [field_separator]] "}" ;

table_accessor = "[" (literal_string | literal_char | digit) "]" ;


function_body = "(" {identifier_list} ")" block ;

function_definition_base = ("function" | "func") identifier function_body ;

function_definition_local = "def" function_definition_base ;

function_definition_global = "def" "global" function_definition_base ;

function_definition_value = ("function" | "func") function_body ;


expression_pre_increment = "++" identifier ;

expression_post_increment = identifier "++" ;

expression_pre_decrement = "--" identifier ;

expression_post_decrement = identifier "--" ;

expression_boolean = expression operator_comparison_binary expression
                        | operator_comparison_unary expression ;

expression = literal
            | function_definition_value
            | array_definition
            | table_definition
            | expression_boolean
            | expression_pre_increment
            | expression_post_increment
            | expression_pre_decrement
            | expression_post_decrement
            ;
            
expression_list = expression [field_separator expression] ;
            

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


is_set_statement = "isSet" "(" identifier ")" ;

function_call_statement = identifier "(" [expression_list] ")" ;


condition_resolvable = expression_boolean | function_call_statement ;


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
            | is_set_statement
            | function_definition_local
            | function_definition_global
            | function_call_statement
            | constant_local_definition
            | constant_global_definition
            | variable_local_definition
            | variable_global_definition
            | variable_local_unset
            | variable_global_unset
            | conditional_iteration_while_statement
            | conditional_iteration_do_statement
            | numeric_iteration_statement
            ;


return_statement = "return" [expression] ;


block = "{" {statement} [return_statement] "}" ;

```