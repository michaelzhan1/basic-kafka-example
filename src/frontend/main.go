package main

import (
	"log"
	"net/http"
)

func main() {
	mux := http.NewServeMux()

	mux.Handle("/", http.FileServer(http.Dir("./public")))

	log.Println("Frontend server is running on http://localhost:8081")
	log.Fatal(http.ListenAndServe(":8081", mux))
}
