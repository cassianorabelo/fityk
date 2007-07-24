// Implementation of class UxdDataSet for reading meta-data and xy-data from 
// Licence: GNU General Public License version 2
// $Id: UxdDataSet.h $

#include "ds_uxd.h"
#include "common.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

bool UxdDataSet::is_filetype() const
{
    return true;
}

void UxdDataSet::load_data() 
{
    init();
    ifstream &f = *p_ifs;

    string line, key, val;
    line_type ln_type;

    // file-scope meta-info
    while (true) {
        skip_invalid_lines(f);
        int pos = f.tellg();
        my_getline(f, line);
        if (str_startwith(line, rg_start_tag)) {
            f.seekg(pos);
            break;
        }
        
        ln_type = get_line_type(line);

        if (LT_KEYVALUE == ln_type) {   // file-level meta key-value line
            parse_line(line, meta_sep, key, val);
            key = ('_' == key[0]) ? key.substr(1) : key;
            add_meta(key, val);
        } else {                        // unkonw line type
            continue;
        }
    }

    // handle ranges
    while (!f.eof()) {
        FixedStepRange *p_rg = new FixedStepRange;
        parse_range(p_rg);
        ranges.push_back(p_rg);
    } 
}


// parse a single range of the file
void UxdDataSet::parse_range(FixedStepRange *p_rg)
{
    ifstream &f = *p_ifs;

    string line;
    // get range-scope meta-info
    while (true) {
        skip_invalid_lines(f);
        int pos = f.tellg();
        my_getline(f, line);
        line_type ln_type = get_line_type(line);
        if (LT_XYDATA == ln_type) {
            f.seekg(pos);
            break;
        }

        if (LT_KEYVALUE == ln_type) {   // range-level meta key-value line
            string key, val;
            parse_line(line, meta_sep, key, val);
            if (key == x_start_key) {
                p_rg->set_x_start(strtod(val.c_str(), NULL));
            } else if (key == x_step_key) {
                p_rg->set_x_step(strtod(val.c_str(), NULL));
            }
            key = ('_' == key[0]) ? key.substr(1) : key;
            p_rg->add_meta(key, val);
        } else {                        // unkonw line type
            continue;
        }
    }

    // get all x-y data
    while (true) {
        if (!skip_invalid_lines(f)) {
            return;
        }  
        int pos = f.tellg();
        my_getline(f, line, false);
        line_type ln_type = get_line_type(line);
        if (str_startwith(line, rg_start_tag)) {
            f.seekg(pos);
            return;                     // new range
        } else if (LT_XYDATA != ln_type) {
            continue;
        }

        for (string::iterator i = line.begin(); i != line.end(); ++i) {
            if (string::npos != data_sep.find(*i)) {
                *i = ' ';
            }
        }
        
        istringstream q(line);
        double d;
        while (q >> d) {
            p_rg->add_y(d);
        }
    }
}

} // end of namespace xylib

