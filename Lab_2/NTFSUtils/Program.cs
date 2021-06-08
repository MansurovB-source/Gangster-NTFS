using System;
using System.Runtime.InteropServices;

namespace NTFSUtils
{
    class Program
    {
        [DllImport("libntfsutil.so.0.0")]
        static extern IntPtr ntfs_init([MarshalAs(UnmanagedType.LPStr)] string filename);

        [DllImport("libntfsutil.so.0.0")]
        static extern int ntfs_close(IntPtr g_info);

        [DllImport("libntfsutil.so.0.0")]
        static extern int free_ls_info(IntPtr first);
        
        [return: MarshalAs(UnmanagedType.LPStr)]
        [DllImport("libntfsutil.so.0.0")]
        static extern string pwd(IntPtr g_info);

        [return: MarshalAs(UnmanagedType.LPStr)]
        [DllImport("libntfsutil.so.0.0")]
        static extern string cd(IntPtr g_info, [MarshalAs(UnmanagedType.LPStr)] string to_path);

        [DllImport("libntfsutil.so.0.0")]
        static extern IntPtr ls(IntPtr g_info, [MarshalAs(UnmanagedType.LPStr)] string to_path);

        [return: MarshalAs(UnmanagedType.LPStr)]
        [DllImport("libntfsutil.so.0.0")]
        static extern string cp(IntPtr g_info, [MarshalAs(UnmanagedType.LPStr)] string from_path,
            [MarshalAs(UnmanagedType.LPStr)] string to_path);
        
        [DllImport("ntfsutil.so.0.0")]
        static extern void print_device();
        
        static void Main(string[] args)
        {
            if (args.Length >= 1 && args[0].Equals("list"))
            {
                Program.print_device();
            }

            if (args.Length >= 2 && args[0].Equals("shell"))
            {
                IntPtr g_info = ntfs_init(args[1]);
                if (g_info == default)
                {
                    Console.WriteLine("Filesystem is not detected");
                    return;
                }
                else
                {
                    Console.WriteLine("NTFS filesystem detected");
                }

                bool exit = false;
                String pwd;
                String[] input;
                String output;
                while (!exit)
                {
                    pwd = Program.pwd(g_info);
                    Console.Write("{0} > ", pwd);
                    input = Console.ReadLine().Split(" ");

                    switch (input[0])
                    {
                        case "exit":
                            exit = true;
                            break;
                        case "help":
                            Console.WriteLine("ls - show working directory elements");
                            Console.WriteLine("cd [directory] - change working directory");
                            Console.WriteLine("pwd - print working directory");
                            Console.WriteLine("cp [directory] [target directory] - copy dir or file from file system");
                            Console.WriteLine("help - list of commands");
                            Console.WriteLine("exit - terminate");
                            break;
                        case "ls":
                            var path = input.Length >= 2 ? input[1] : ".";
                            LsInfo lsInfo = (LsInfo) Marshal.PtrToStructure(Program.ls(g_info, path), typeof(LsInfo));
                            //LsInfo lsInfo = (LsInfo) Program.ls(g_info, path);
                            lsInfo = (LsInfo) Marshal.PtrToStructure(lsInfo.Ptr, typeof(LsInfo));
                            
                            while (lsInfo.Ptr != default)
                            {
                                switch (lsInfo.Type)
                                {
                                    case 1:
                                        Console.WriteLine("Dir: {0}", lsInfo.Filename);
                                        break;
                                    default:
                                        Console.WriteLine("File: {0}", lsInfo.Filename);
                                        break;
                                }
                                lsInfo = (LsInfo) Marshal.PtrToStructure(lsInfo.Ptr, typeof(LsInfo));
                            }
                            //IntPtr info = Program.ls(g_info, path);
                            //TODO
                            break;
                        case "pwd":
                            output = Program.pwd(g_info);
                            Console.WriteLine(output);
                            break;
                        case "cd":
                            if (input.Length >= 2)
                            {
                                output = Program.cd(g_info, input[1]);
                                Console.WriteLine(output);
                            }
                            else
                            {
                                Console.WriteLine("cd command require path argument");
                            }
                            break;
                        case "cp":
                            switch (input.Length)
                            {
                                case 3:
                                    output = Program.cp(g_info, input[1], input[2]);
                                    Console.WriteLine(output);
                                    break;
                                case 2:
                                    Console.WriteLine("cp command requires \"out_path\" argument");
                                    break;
                                default:
                                    Console.WriteLine("cp command requires \"path\" and \"out_path\" arguments");
                                    break;
                            }
                            break;
                        default:
                            Console.WriteLine("wrong command. Enter \"help\" to get more information");
                            break;
                    }
                }
                ntfs_close(g_info);
                return;
            }
            Console.WriteLine("Incorrect command line arguments. Run with \"help\" argument to get help");
        }
    }
}