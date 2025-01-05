def main():
    print("Hello World")
    driver_handle = open("/proc/driver_name")
    msg_frm_kernel = driver_handle.readline()
    print(msg_frm_kernel)
    driver_handle.close
    return
main()