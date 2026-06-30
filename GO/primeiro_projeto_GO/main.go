package main

import "fmt"

func main() {
	fmt.Println("Olá Go!")
	const teste = 10
	fmt.Print(teste)
	soma(10, 20)

}

func soma(a, b int) {
	fmt.Println(a + b)
}
