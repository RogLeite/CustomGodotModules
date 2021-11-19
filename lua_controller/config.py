# config.py

# Dada a plataforma e o ambiente de build, confirma se pode compilar
def can_build (env, platform):
    return True

# Não sei o que é para fazer aqui
def configure (env):
    pass

# Sistema de compilação usa para determinar diretório da documentação (default é "doc/classes")
def get_doc_path ():
    return "doc_classes"

# Explicita ao sistema de compilação quais classes pertencem ao módulo
def get_doc_classes ():
    return [
        "LuaController"
    ]