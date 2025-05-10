import random
import time

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

def save_commands_to_file(commands, filename):
    """
    将命令保存为指定格式的文件
    格式：[地址]=数据|序号|标识|1|
    """
    with open(filename, 'w') as f:
        # 写入文件头 [BATCHSEND]
        f.write("[BATCHSEND]\n")

        # 先写入指令头部
        addr = f"{random.randint(0x67F00000, 0x67FFFFFF):08X}"
        f.write(f"{addr}=7F|00000501|1000|1|\n")
        
        # 生成序列号基数
        # seq_base = f"{random.randint(0, 99):02d}"
        seq_base = 0
        
        for i, command in enumerate(commands, 1):
            # 拆分命令的部分
            parts = command.split("\n")
            
            # 处理头部指令 "11 EE"
            addr = f"{random.randint(0x67F00000, 0x67FFFFFF):08X}"
            # seq_num = f"{seq_base}{i:02d}0501"
            seq_num = f"{seq_base:04d}0501"
            seq_num += 1
            f.write(f"{addr}=11EE|{seq_num}|1000|1|\n")
            
            # 处理地址和校验和部分
            if len(parts) > 1:
                addr_data = parts[1].replace(" ", "")
                addr = f"{random.randint(0x67F00000, 0x67FFFFFF):08X}"
                # seq_num = f"{seq_base}{i*2:02d}0501"
                seq_num = f"{seq_base:04d}0501"
                
                f.write(f"{addr}={addr_data}|{seq_num}|1000|1|\n")
            
            # 处理长度部分
            if len(parts) > 2:
                length_data = parts[2].replace(" ", "")
                addr = f"{random.randint(0x67F00000, 0x67FFFFFF):08X}"
                seq_num = f"{seq_base}{i*2+1:02d}0501"
                f.write(f"{addr}={length_data}|{seq_num}|1000|1|\n")

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
    for command in commands:
        print(command)
        # 每条指令之间添加空行
        print("...")
    print("...")
    
    # 打印统计信息
    print(f"\n总共需要发送 {len(commands)} 条指令")
    
    # 保存指令到文件
    output_file = "d:/dateXHY/STM32CubeMX/homework6/commands.txt"
    save_commands_to_file(commands, output_file)
    print(f"\n指令已保存到文件: {output_file}")