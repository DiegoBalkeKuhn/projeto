package atividadeLorenzon;

public class Empresa {
String nome;
String cnpj;  
Endereco endereco;
Pessoa ceo;

 void imprimirDados() {
     System.out.println("Nome: " + nome);
     System.out.println("CNPJ: " + cnpj);
     System.out.print("Endereço: ");
     endereco.imprimirEndereco();
     ceo.imprimirDadosPessoais();
 }

 public Empresa(String nome, String cnpj, Endereco endereco, Pessoa ceo) {
     this.nome = nome;
     this.cnpj = cnpj;
     this.endereco = endereco;
     this.ceo = ceo;
 }
}
