#ifndef INPUT_H
#define INPUT_H

/*
 * get_user_input() is used to safely get input from the user.
 * Parameters:
 * 1. buff: A buffer in which the user input will be returned.
 * 2. buff_len: The length of the buffer in which the input is to be returned and it also dictates the maximum size to get from user.
 * 3. prompt: A string that will be shown to user with a colon added at the end when asking for input.
 * 4. confirm: A number to indicate if the user should be prompted once again and confirm if the inputs match.
 *    1 indicates that confirmation is required
 *    any other value will indicate confirmation is not required.
 * 5. secret: A number used to indicate whether 'echo' property should be removed from the terminal while the user types which is useful for passwords.
 *    1 indicates that echo property should be removed.
 *    any other value indicates that the terminal should be left as it is.
 *
 * Returns SUCCESS_OP on success and GENERAL_ERROR on failure.
 */
int get_user_input(char *buff, int buff_len, const char *prompt, int confirm, int secret);

#endif
