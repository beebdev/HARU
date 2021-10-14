#include "subsequence_search.h"
#include "AESL_pkg.h"

using namespace std;

namespace ap_rtl {

void subsequence_search::thread_hdltv_gen() {
    const char* dump_tv = std::getenv("AP_WRITE_TV");
    if (!(dump_tv && string(dump_tv) == "on")) return;

    wait();

    mHdltvinHandle << "[ " << endl;
    mHdltvoutHandle << "[ " << endl;
    int ap_cycleNo = 0;
    while (1) {
        wait();
        const char* mComma = ap_cycleNo == 0 ? " " : ", " ;
        mHdltvinHandle << mComma << "{"  <<  " \"ap_rst\" :  \"" << ap_rst.read() << "\" ";
        mHdltvinHandle << " , " <<  " \"ap_start\" :  \"" << ap_start.read() << "\" ";
        mHdltvoutHandle << mComma << "{"  <<  " \"ap_done\" :  \"" << ap_done.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"ap_idle\" :  \"" << ap_idle.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"ap_ready\" :  \"" << ap_ready.read() << "\" ";
        mHdltvinHandle << " , " <<  " \"agg_result_dist_i\" :  \"" << agg_result_dist_i.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"agg_result_dist_o\" :  \"" << agg_result_dist_o.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"agg_result_dist_o_ap_vld\" :  \"" << agg_result_dist_o_ap_vld.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"agg_result_end_position\" :  \"" << agg_result_end_position.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"agg_result_end_position_ap_vld\" :  \"" << agg_result_end_position_ap_vld.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"x_address0\" :  \"" << x_address0.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"x_ce0\" :  \"" << x_ce0.read() << "\" ";
        mHdltvinHandle << " , " <<  " \"x_q0\" :  \"" << x_q0.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"x_address1\" :  \"" << x_address1.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"x_ce1\" :  \"" << x_ce1.read() << "\" ";
        mHdltvinHandle << " , " <<  " \"x_q1\" :  \"" << x_q1.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"y_address0\" :  \"" << y_address0.read() << "\" ";
        mHdltvoutHandle << " , " <<  " \"y_ce0\" :  \"" << y_ce0.read() << "\" ";
        mHdltvinHandle << " , " <<  " \"y_q0\" :  \"" << y_q0.read() << "\" ";
        mHdltvinHandle << "}" << std::endl;
        mHdltvoutHandle << "}" << std::endl;
        ap_cycleNo++;
    }
}

}

