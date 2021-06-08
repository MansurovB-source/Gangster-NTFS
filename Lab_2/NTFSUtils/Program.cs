using System;
using System.Runtime.InteropServices;

namespace NTFSUtils
{
    class Program
    {
        [DllImport("libntfsutil.so.0.0")]
        static extern IntPtr ntfs_init([MarshalAs(UnmanagedType.LPStr)] string filename);

        [DllImport("libntfsutil.so.0.0")]
        static extern int ntfs_close(IntPtr gInfo);

        [DllImport("libntfsutil.so.0.0")]
        static extern int free_ls_info(IntPtr first);

        [return: MarshalAs(UnmanagedType.LPStr)]
        [DllImport("libntfsutil.so.0.0")]
        static extern string pwd(IntPtr gInfo);

        [return: MarshalAs(UnmanagedType.LPStr)]
        [DllImport("libntfsutil.so.0.0")]
        static extern string cd(IntPtr gInfo, [MarshalAs(UnmanagedType.LPStr)] string toPath);

        [DllImport("libntfsutil.so.0.0")]
        static extern IntPtr ls(IntPtr gInfo, [MarshalAs(UnmanagedType.LPStr)] string toPath);

        [return: MarshalAs(UnmanagedType.LPStr)]
        [DllImport("libntfsutil.so.0.0")]
        static extern string cp(IntPtr gInfo, [MarshalAs(UnmanagedType.LPStr)] string fromPath,
            [MarshalAs(UnmanagedType.LPStr)] string toPath);

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
                IntPtr gInfo = ntfs_init(args[1]);
                if (gInfo == default)
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
                IntPtr intPtr = default;

                while (!exit)
                {
                    pwd = Program.pwd(gInfo);
                    Console.Write("{0} > ", pwd);
                    input = Console.ReadLine()?.Split(" ");

                    if (input != null)
                        switch (input[0])
                        {
                            case "exit":
                                exit = true;
                                break;
                            case "help":
                                Console.WriteLine("ls - show working directory elements");
                                Console.WriteLine("cd [directory] - change working directory");
                                Console.WriteLine("pwd - print working directory");
                                Console.WriteLine(
                                    "cp [directory] [target directory] - copy dir or file from file system");
                                Console.WriteLine("help - list of commands");
                                Console.WriteLine("exit - terminate");
                                break;
                            case "ls":
                                var path = input.Length >= 2 ? input[1] : ".";
                                LsInfo lsInfo;
                                if ((intPtr = Program.ls(gInfo, path)) != default)
                                {
                                    lsInfo = (LsInfo) Marshal.PtrToStructure(intPtr, typeof(LsInfo));
                                    if (lsInfo != default)
                                    {
                                        lsInfo = (LsInfo) Marshal.PtrToStructure(lsInfo.Ptr, typeof(LsInfo));
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    break;
                                }

                                while (lsInfo != null && lsInfo.Ptr != default)
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
                                output = Program.pwd(gInfo);
                                Console.WriteLine(output);
                                break;
                            case "cd":
                                if (input.Length >= 2)
                                {
                                    output = Program.cd(gInfo, input[1]);
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
                                        output = Program.cp(gInfo, input[1], input[2]);
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
                ntfs_close(gInfo);
                if (intPtr != default)
                {
                    free_ls_info(intPtr);
                }
                return;
            }
            Console.WriteLine("Incorrect command line arguments. Run with \"help\" argument to get help");
        }
    }
}