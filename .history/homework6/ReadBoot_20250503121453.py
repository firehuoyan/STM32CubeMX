def generate_read_commands(start_addr, end_addr, max_bytes=256):
    """
    生成一系列串口读取指令，从start_addr读取到end_addr
    每次最多读取max_bytes个字节
    """
    commands = []
    current_addr = start_addr
    
    while current_addr < end_addr:
        # 计算本次读取的字节数
        bytes_left = end_addr - current_addr + 1
        bytes_to_read = min(bytes_left, max_bytes)
        
        # 如果刚好是256字节，使用FF 00表示
        if bytes_to_read == 256:
            length_str = "FF 00"
        else:
            # 否则，取低8位和补码
            length = bytes_to_read - 1  # 实际长度减1
            length_complement = (~length) & 0xFF  # 取补码
            length_str = f"{length:02X} {length_complement:02X}"
        
        # 地址分割为4字节
        addr_bytes = [
            (current_addr >> 24) & 0xFF,
            (current_addr >> 16) & 0xFF,
            (current_addr >> 8) & 0xFF,
            current_addr & 0xFF
        ]
                
        # 计算地址的校验和（使用异或，不取反）
        addr_checksum = 0
        for b in addr_bytes:
            addr_checksum ^= b

        # 使用校验和的值
        addr_complement = addr_checksum
        
        # 生成指令
        addr_str = " ".join(f"{b:02X}" for b in addr_bytes)
        command = f"11 EE\n{addr_str} {addr_complement:02X}\n{length_str}"
        commands.append(command)
        
        # 更新地址
        current_addr += bytes_to_read
    
    return commands

if __name__ == "__main__":
    # 起始地址 0x1FFF0000，结束地址 0x1FFF7A0F
    start_address = 0x1FFF0000
    end_address = 0x1FFF7B00
    
    commands = generate_read_commands(start_address, end_address)
    
    # 测试头两条指令
    print(f"第一条: {commands[0]}")
    print(f"第二条: {commands[1]}")
    # 打印所有指令
    print("\n所有指令:")
    
    print("...")
    
    # 打印统计信息
    print(f"\n总共需要发送 {len(commands)} 条指令")