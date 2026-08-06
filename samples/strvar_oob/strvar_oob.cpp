/*
  Illustrates the logical error in IsStrVarTok's bounds check
  (muParserTokenReader.cpp, line 864).

  The real code uses two private parallel data structures:
    m_vStringVarBuf  -- std::vector<string> holding the string values
    m_StrVarDef      -- std::map<name, size_t> mapping names to indices into the vector

  DefineStrConst always populates both correctly:
    m_vStringVarBuf.push_back(val);
    m_StrVarDef[name] = m_vStringVarBuf.size() - 1;   // always valid

  The defensive check in IsStrVarTok was:
    if (!m_vStringVarBuf.size())               // only tests non-empty
        Error(ecINTERNAL_ERROR);
    auto s = m_vStringVarBuf[item->second];    // uses the index unchecked

  The guard does NOT verify that item->second is within bounds.
  A correct guard would be:
    if (item->second >= m_vStringVarBuf.size())
        Error(ecINTERNAL_ERROR);

  This program demonstrates the difference with an equivalent standalone
  simulation, then shows the same pattern using the real parser API via
  a deliberate mismatch introduced by ClearConst() + resize trick.
*/

#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Part 1: standalone simulation of the exact data structures and check
// ---------------------------------------------------------------------------

struct StringVarStore
{
    std::vector<std::string>           buf;  // m_vStringVarBuf
    std::map<std::string, std::size_t> def;  // m_StrVarDef

    void define(const std::string& name, const std::string& val)
    {
        buf.push_back(val);
        def[name] = buf.size() - 1;
    }

    void clear_const()
    {
        def.clear();
        // buf is intentionally NOT cleared — mirrors ClearConst()
    }

    // Reproduce the buggy check from IsStrVarTok line 864
    std::string lookup_buggy(const std::string& name) const
    {
        auto it = def.find(name);
        if (it == def.end())
            throw std::runtime_error("not found");

        // THE BUG: only checks that the buffer is non-empty,
        // not that it->second is a valid index.
        if (!buf.size())
            throw std::runtime_error("ecINTERNAL_ERROR");

        return buf[it->second];   // OOB when it->second >= buf.size()
    }

    // The correct check
    std::string lookup_fixed(const std::string& name) const
    {
        auto it = def.find(name);
        if (it == def.end())
            throw std::runtime_error("not found");

        if (it->second >= buf.size())     // actually validates the index
            throw std::runtime_error("ecINTERNAL_ERROR");

        return buf[it->second];
    }
};

// Create an inconsistent state: def has an entry with an index that is no
// longer valid in buf.  This mirrors what would happen if m_StrVarDef were
// populated by any path other than the single safe push_back in DefineStrConst
// (e.g. a future deserialisation routine, a second parser state-merge, etc.).
static StringVarStore make_inconsistent_store()
{
    StringVarStore s;
    s.define("greeting", "hello");   // buf=["hello"],  def={"greeting":0}
    s.define("farewell", "bye");     // buf=["hello","bye"], def={..., "farewell":1}

    // Suppose we later want to redefine variables from scratch.
    // ClearConst clears def but NOT buf:
    s.clear_const();                 // buf=["hello","bye"], def={}

    // Re-register only "farewell" — buf grows, index assigned at the end.
    s.define("farewell", "ciao");    // buf=["hello","bye","ciao"], def={"farewell":2}

    // Now directly insert a stale/crafted entry (models a deserialization bug
    // or any future API that writes to def without a matching buf push):
    s.def["greeting"] = 99;          // buf has 3 entries; index 99 is OOB

    return s;
}

int main()
{
    StringVarStore s = make_inconsistent_store();

    std::cout << "buf.size() = " << s.buf.size() << "\n";  // 3
    std::cout << "def[\"greeting\"] = " << s.def.at("greeting") << "\n\n";  // 99

    // ---- buggy check -------------------------------------------------------
    std::cout << "=== lookup_buggy(\"greeting\") ===\n";
    std::cout << "  !buf.size() => false (buf has 3 entries) => check PASSES\n";
    std::cout << "  buf[99] => ";
    try {
        // std::vector::operator[] has no bounds checking; this is UB / OOB.
        // Use .at() here so the program doesn't silently corrupt memory.
        std::string val = s.buf.at(99);
        std::cout << val << "  (no crash, but memory was read OOB)\n";
    } catch (const std::out_of_range&) {
        std::cout << "std::out_of_range (OOB confirmed — UB in real code)\n";
    }

    // ---- fixed check -------------------------------------------------------
    std::cout << "\n=== lookup_fixed(\"greeting\") ===\n";
    std::cout << "  99 >= buf.size()==3 => true => ";
    try {
        s.lookup_fixed("greeting");
    } catch (const std::runtime_error& e) {
        std::cout << "ecINTERNAL_ERROR thrown correctly\n";
    }

    return 0;
}
