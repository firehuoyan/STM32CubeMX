# filepath: [校验.py](http://_vscodecontentref_/1)
def hex_xor(input_hex):
    hex_bytes = input_hex.strip().split()
    result = 0
    for byte in hex_bytes:
        result ^= int(byte, 16)
    return format(result, '02x')

def get_complement(hex_byte):
    # 将十六进制字节转为整数
    value = int(hex_byte, 16)
    # 按位取反 (异或 0xFF)
    complement = value ^ 0xFF
    return format(complement, '02x')

if __name__ == "__main__":
    while True:
        user_input = input("请输入十六进制字节（用空格分隔）：")
        if user_input.lower() == 'exit':
            break
        
        hex_bytes = user_input.strip().split()
        if len(hex_bytes) == 1:
            # 只有一个字节时，计算反码(按位取反)
            output = get_complement(hex_bytes[0])
            print(f"字节 {hex_bytes[0]} 的反码为: {output}")
        else:
            # 多个字节时，执行异或运算
            output = hex_xor(user_input)
            print(f"累计异或结果为: {output}")