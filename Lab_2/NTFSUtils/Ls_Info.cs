using System;
using System.Runtime.InteropServices;

namespace NTFSUtils
{
    [StructLayout(LayoutKind.Sequential)]
    public class LsInfo
    {
        private string filename;
        private int type;
        private IntPtr _ptr;

        public string Filename
        {
            get => filename;
            set => filename = value;
        }

        public int Type
        {
            get => type;
            set => type = value;
        }

        public IntPtr Ptr
        {
            get => _ptr;
            set => _ptr = value;
        }
    }
}