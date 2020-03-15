

/* These prevent symbols being visible outside the DLL, which has the
 * useful effect of preventing the use of PLT when they are called,
 * see https://stackoverflow.com/questions/51573771/call-a-function-in-another-object-file-without-using-plt-within-a-shared-library */
.hidden x86_switch_to_user
.hidden x86_switch_from_user
.hidden x86_startup_stage_2
.hidden x86_other_context
.hidden x86_size_of_red_zone
.hidden x86_size_of_call_instruction
.hidden x86_bp_trap
.hidden x86_is_branch_taken
.hidden x86_asm_begin
.hidden x86_asm_end
.hidden _x86_switch_to_user
.hidden _x86_switch_from_user
.hidden _x86_startup_stage_2
.hidden _x86_other_context
.hidden _x86_size_of_red_zone
.hidden _x86_size_of_call_instruction
.hidden _x86_bp_trap
.hidden _x86_is_branch_taken
.hidden _x86_asm_begin
.hidden _x86_asm_end
 
 
 
