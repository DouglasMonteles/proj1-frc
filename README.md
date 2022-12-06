# Universidade de Brasília / Fga Engenharia de Software
## Disciplina: *Fundamentos de Redes de Computadores - Prof. Fernando W. Cruz*

## 1º. Projeto

### 1. Título: Simulação do modelo de Referência OSI pela construção de um protótipo de camada de enlace de dados.

### Primeira parte (simplex)

- Acesse o código da primeira parte neste link: <https://github.com/DouglasMonteles/proj1-frc/tree/proj-simplex>

#### Como executar:

```
# Compile o projeto
make

# Execute o servidor como receptor da mensagem
./bin/projeto-1 0.0.0.0 5000 192.168.15.10 5000 RECEIVER

# Execute o servidor como remetente da mensagem
./bin/projeto-1 0.0.0.0 5000 192.168.15.10 5000 SENDER

# Execute a interface 
./bin/projeto-1 interface
```

### Segunda Parte (half-duplex)

- Acesse o código da primeira parte neste link: <https://github.com/DouglasMonteles/proj1-frc/tree/proj-halfduplex>

#### Como executar:

```
# Compile o projeto
make

# Execute o servidor como remetente / receptor da mensagem
./bin/projeto-1 server 5000 192.168.15.10 5000 112

# Execute a interface 
./bin/projeto-1 interface
```