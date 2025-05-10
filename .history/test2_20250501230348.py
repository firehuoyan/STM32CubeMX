def hex_xor(input_hex):
    hex_bytes = input_hex.strip().split()
    result = 0
    for byte in hex_bytes:
        result ^= int(byte, 16)
    return format(result, '02x')

if __name__ == "__main__":
    while True:
        try:
            user_input = input("请输入十六进制字节（用空格分隔）：")
            output = hex_xor(user_input)
            print(f"累计异或结果为: {output}")
        except ValueError:
            print("输入无效，请确保输入的是十六进制字节，并用空格分隔。")
        except KeyboardInterrupt:
            print("\n程序已终止。")
            break
        except Exception as e:
            print(f"发生错误: {e}")
    user_input = input("请输入十六进制字节（用空格分隔）：")
    output = hex_xor(user_input)
    print(f"累计异或结果为: {output}")