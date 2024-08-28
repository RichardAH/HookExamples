#include "hookapi.h"

int64_t hook(uint32_t reserved) {

    uint8_t user_acc_id[20];
    otxn_field(SBUF(user_acc_id), sfAccount);

    uint8_t hook_acc_id[20];
    hook_account(hook_acc_id, 20);

    int8_t equal = 0; BUFFER_EQUAL(equal, hook_acc_id, user_acc_id, 20);
    if(equal)
        accept(SBUF("Pre-Authorize: Outgoing Transaction."), 1);    

    uint8_t publickey[1] = { 0x50U };
    uint8_t publickey_ptr[33];
    if(hook_param(publickey_ptr, 33, publickey, 1) != 33)
        rollback(SBUF("Pre-Authorize: PublicKey not set as Hook parameter"), 2);  

    uint8_t inputKey[1] = { 0x49U };
    uint8_t input[100];
    if(otxn_param(input, 100, inputKey, 1) != 100)
        rollback(SBUF("Pre-Authorize: Blob must be 64 + 20 + 8 + 8 = 100 bytes."), 3);   

    if(util_verify(input + 64, 36, input, 64, publickey_ptr, 33) != 1) {
        rollback(SBUF("Pre-Authorize: Unauthorized Transaction."), 4);   
    }      

    BUFFER_EQUAL(equal, input + 64, user_acc_id, 20);
    if(!equal)
        rollback(SBUF("Pre-Authorize: Unauthorized Account."), 5);  

    int64_t amount   = *((int64_t*)input[84]);
    int64_t txn_amount;
    otxn_field(SVAR(txn_amount), sfAmount);    
    if(float_compare(amount, txn_amount, COMPARE_EQUAL) == 0)
        rollback(SBUF("Pre-Authorize: Unauthorized Amount."), 6);

    int64_t sequence = *((int64_t*)input[92]);
    int64_t user_acc_seq;
    otxn_field(SVAR(user_acc_seq), sfSequence);
    if(float_compare(sequence, user_acc_seq, COMPARE_EQUAL) == 0)
        rollback(SBUF("Pre-Authorize: Authorization Previously Used."), 7);
  
    accept(SBUF("Pre-Authorize: Payment Verified and Accepted."), 8);
    _g(1,1);
    return 0;    
}