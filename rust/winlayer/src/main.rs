extern crate winapi;
extern crate user32;
use std::ffi::OsStr;
use std::io::Error;
use std::iter::once;
use std::os::windows::ffi::OsStrExt;
use std::ptr::null_mut;

fn main() {
    let msg = "Hello, world!";
    let classname = "Xi-WinLayer";
    let wideMSG: Vec<u16> = OsStr::new(msg).encode_wide().chain(once(0)).collect();
    let wideClassName: Vec<u16> OsStr::new(classname).encode_wide().chain(once(0)).collect(); 
    let window = WNDCLASSW {
      hInstance:  kernel32::GetModuleHandleW(ptr::null()),
      lpfnWndProc: win32MainWindowCallback,
      lpszClassName: wideClassName
    }
    let ret = unsafe {
        user32::RegisterClassW(window) //returns unsigned short
    };
    if ret == 0 {
        println!("Failed: {:?}", Error::last_os_error());
    }
    while {
      let msg: LPMSG;
      let msgResult = GetMessageW( msg, window, 0, 0);
      if msgResult > 0 {
        
        
      } else {
        println!("Msg Failed: {:?}", Error::last_os_error());
      }
    }
}


fn win32MainWindowCallback( window: HWND, message: UINT, wparam: WPARAM, lparam: LPARAM ) -> LRESULT {
  let result: LRESULT = 0;
    //TODO handle messages
  result
}