define print_shared_ptr
  print "Usage: print_shared_ptr initial_config.admin_.address_"
  set $ptr = $arg0
  set $m_ptr = $ptr._M_ptr
  ptype $m_ptr
  set print object on
  print $m_ptr
  print *($m_ptr)
end
