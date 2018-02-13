#include "./token.hpp"

namespace c4 {
namespace tpl {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void TokenBase::parse(cspan *rem, TplLocation *curr_pos)
{
    // todo deal with nested tokens
    auto const &s = stoken(), &e = etoken();
    C4_ASSERT(rem->begins_with(s));
    m_start = *curr_pos;
    auto pos = rem->find(e);
    C4_ASSERT(pos != npos);
    C4_ASSERT(pos + e.len <= rem->len);
    m_full_text = rem->subspan(0, pos + e.len);
    m_interior_text = m_full_text.subspan(e.len, m_full_text.len - e.len - s.len);

    auto &rp = curr_pos->m_rope_pos;
    C4_ASSERT(curr_pos->m_rope->get(rp.entry)->s.len >= m_full_text.len);
    rp.entry = curr_pos->m_rope->replace(rp.entry, rp.i, m_full_text.len, this->marker());
    m_rope_entry = rp.entry;
    rp.entry = curr_pos->m_rope->next(rp.entry);
    rp.i = 0;
    m_end = *curr_pos;

    *rem = rem->subspan(m_full_text.len);
}

bool TokenBase::eval(NodeRef const& root, cspan key, cspan *value) const
{
    C4_ASSERT(root.valid());
    NodeRef n = root;
    do {
        auto pos = key.find('.');
        if(pos != npos)
        {
            cspan left = key.left_of(pos);
            n = n.find_child(left);
            key = key.right_of(pos);
        }
        else
        {
            pos = key.find('[');
            if(pos != npos)
            {
                cspan left = key.left_of(pos);
                if( ! left.empty())
                {
                    n = n.find_child(left);
                    if( ! n.valid())
                    {
                        break;
                    }
                    key = key.right_of(pos);
                    pos = key.find(']');
                    C4_ASSERT(pos != npos);
                    cspan subkey = key.left_of(pos);
                    key = key.right_of(pos);
                    if( ! this->eval(n, subkey, &subkey))
                    {
                        break;
                    }
                    else
                    {
                        *value = subkey;
                        return true;
                    }
                }
            }
            else
            {
                if(key.begins_with_any("0123456789"))
                {
                    size_t num;
                    bool ret = from_str(key, &num);
                    if(ret)
                    {
                        if(n.num_children() >= num)
                        {
                            n = n[num];
                            key.clear();
                        }
                    }
                }
                else
                {
                    n = n.find_child(key);
                    key.clear();
                }
            }
        }
    } while( ! key.empty() && n.valid());

    if(n.valid())
    {
        *value = n.val();
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void IfCondition::eval(NodeRef const& root)
{
    if( ! m_arg.empty())
    {
        if( ! m_token->eval(root, m_arg, &m_argval))
        {
            m_argval = m_arg;
        }
    }
    if( ! m_cmp.empty())
    {
        if( ! m_token->eval(root, m_cmp, &m_cmpval))
        {
            m_cmpval = m_cmp;
        }
    }
}

bool IfCondition::resolve(NodeRef const& root)
{
    eval(root);
    switch(m_ctype)
    {
    case ARG:        return ! m_argval.empty();
    case ARG_EQ_CMP: return   m_argval.compare(m_cmpval) == 0;
    case ARG_NE_CMP: return   m_argval.compare(m_cmpval) != 0;
    case ARG_GE_CMP: return   m_argval.compare(m_cmpval) >= 0;
    case ARG_GT_CMP: return   m_argval.compare(m_cmpval) >  0;
    case ARG_LE_CMP: return   m_argval.compare(m_cmpval) <= 0;
    case ARG_LT_CMP: return   m_argval.compare(m_cmpval) <  0;
    case ARG_IN_CMP:
    case ARG_NOT_IN_CMP:
    {
        bool in_cmp;
        if(root.is_map())
        {
            in_cmp = root.find_child(m_arg).valid();
        }
        else if(root.is_seq())
        {
            for(auto &n : root.children())
            {
                if(n.is_val() && n.val() == m_arg) in_cmp = true;
            }
            in_cmp = false;
        }
        return m_ctype == ARG_IN_CMP ? in_cmp : ! in_cmp;
    }
    // do the other condition types
    default:
        C4_ERROR("unknown condition type");
        break;
    }
    C4_ERROR("never reach");
    return false;
}

void IfCondition::parse()
{
    C4_ASSERT(m_str.first_of("{}*") == npos);
    auto pos = m_str.first_of('<');
    if(pos != npos)
    {
        C4_ASSERT(pos+2 < m_str.len);
        if(m_str[pos+1] == '=')
        {
            m_ctype = ARG_LE_CMP;
            m_arg = m_str.left_of(pos).trim(' ');
            m_cmp = m_str.right_of(pos+1).trim(' ');
        }
        else
        {
            m_ctype = ARG_LT_CMP;
            m_arg = m_str.left_of(pos).trim(' ');
            m_cmp = m_str.right_of(pos+1).trim(' ');
        }
        return;
    }

    pos = m_str.first_of('>');
    if(pos != npos)
    {
        C4_ASSERT(pos+2 < m_str.len);
        if(m_str[pos+1] == '=')
        {
            m_ctype = ARG_GE_CMP;
            m_arg = m_str.left_of(pos).trim(' ');
            m_cmp = m_str.right_of(pos+1).trim(' ');
        }
        else
        {
            m_ctype = ARG_GT_CMP;
            m_arg = m_str.left_of(pos).trim(' ');
            m_cmp = m_str.right_of(pos+1).trim(' ');
        }
        return;
    }

    pos = m_str.first_of('!');
    if(pos != npos) // !=
    {
        C4_ASSERT(pos+2 < m_str.len);
        C4_ASSERT(m_str[pos+1] == '=');
        m_ctype = ARG_NE_CMP;
        m_arg = m_str.left_of(pos).trim(' ');
        m_cmp = m_str.right_of(pos+1).trim(' ');
        return;
    }

    pos = m_str.first_of('=');
    if(pos != npos) // ==
    {
        C4_ASSERT(pos+2 < m_str.len);
        C4_ASSERT(m_str[pos+1] == '=');
        m_ctype = ARG_EQ_CMP;
        m_arg = m_str.left_of(pos).trim(' ');
        m_cmp = m_str.right_of(pos+1).trim(' ');
        return;
    }

    pos = m_str.find(" not in ");
    if(pos != npos) //
    {
        m_ctype = ARG_NOT_IN_CMP;
        m_arg = m_str.left_of(pos).trim(' ');
        m_cmp = m_str.right_of(pos+8).trim(' ');
        return;
    }

    pos = m_str.find(" in ");
    if(pos != npos) //
    {
        m_ctype = ARG_IN_CMP;
        m_arg = m_str.left_of(pos).trim(' ');
        m_cmp = m_str.right_of(pos+4).trim(' ');
        return;
    }

    m_ctype = ARG;
    m_arg = m_str.trim(' ');
    m_cmp.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void TokenIf::parse(cspan *rem, TplLocation *curr_pos)
{
    base_type::parse(rem, curr_pos);

    // scan the condition
    cspan s = m_full_text;
    cspan c = _scan_condition(stoken(), &s);
    m_cond_blocks.emplace_back();
    auto *cb = &m_cond_blocks.back();
    cb->condition.init(this, c);

    // scan the branches
    while(1)
    {
        if(s.empty()) break;

        auto pos = s.find("{% if ");
        if(pos != npos) // there's a nested if - skip until it ends
        {
            s = s.subspan(pos);
            pos = s.find(etoken());
            C4_ASSERT(pos != npos);
            s = s.subspan(pos + etoken().len);
        }

        pos = s.find("{% else %}");
        if(pos != npos)
        {
            cb->block = s.subspan(0, pos);
            s = s.subspan(pos + 10); // 10==strlen("{% else %}")
            break;
        }

        pos = s.find("{% elif");
        if(pos == npos)
        {
            pos = s.find(etoken());
            C4_ASSERT(pos != npos);
            cb->block = s.left_of(pos);
            s.clear();
            break;
        }

        cb->block = s.subspan(0, pos);
        s = s.subspan(pos);
        m_cond_blocks.emplace_back();
        cb = &m_cond_blocks.back();
        auto cond = _scan_condition("{% elif ", &s);
        cb->condition.init(this, cond);
    }

    m_else_block = s.trim("\r\n");
    for(auto &cond : m_cond_blocks)
    {
        cond.block = cond.block.trim("\r\n");
    }
}

cspan TokenIf::_scan_condition(cspan token, cspan *s)
{
    C4_ASSERT(s->begins_with(token));
    auto pos = s->find("%}"); // this is where the {% if ... %} tag ends
    C4_ASSERT(pos != npos);
    cspan c = s->range(token.len, pos);
    c = c.trim(' ');
    *s = s->subspan(pos + 2);
    return c;
}

bool TokenIf::resolve(NodeRef const& root, cspan *value) const
{
    for(auto const& cb : m_cond_blocks)
    {
        if(cb.condition.resolve(root))
        {
            *value = cb.block;
            return true;
        }
    }
    *value = m_else_block;
    return true;
}

} // namespace tpl
} // namespace c4