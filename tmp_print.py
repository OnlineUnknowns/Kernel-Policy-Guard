from pathlib import Path
files = [
    'chronos_main.c',
    'src/hooks/inode_hooks.c',
    'src/hooks/ipc_hooks.c',
    'src/hooks/net_hooks.c',
    'src/hooks/task_hooks.c',
    'src/cluster_sync.c',
    'src/abi_handshake.c',
    'telemetry/audit_logger.c'
]
for f in files:
    path = Path(f)
    print(f'===== {f} =====')
    for i, line in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
        print(f'{i:4}: {line}')
    print()
