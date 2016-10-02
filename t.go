package main

import (
	"fmt"
	"time"

	"github.com/Centny/gwf/util"
)

type RequestVars struct {
	Oid           string `json:"oid"`
	Size          int64  `json:"size"`
	User          string `json:"user"`
	Password      string `json:"password"`
	Repo          string `json:"repo"`
	Authorization string `json:"authorization"`
}

type BatchVars struct {
	Transfers []string       `json:"transfers"`
	Operation string         `json:"operation"`
	Objects   []*RequestVars `json:"objects"`
}

type BatchResponse struct {
	Transfer string            `json:"transfer,omitempty"`
	Objects  []*Representation `json:"objects"`
}

// Representation is object medata as seen by clients of the lfs server.
type Representation struct {
	Oid     string           `json:"oid"`
	Size    int64            `json:"size"`
	Actions map[string]*link `json:"actions"`
	Error   *ObjectError     `json:"error,omitempty"`
}

type ObjectError struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
}

type link struct {
	Href      string            `json:"href"`
	Header    map[string]string `json:"header,omitempty"`
	ExpiresAt time.Time         `json:"expires_at,omitempty"`
}

func main() {
	fmt.Println(util.S2Json(&BatchVars{
		Transfers: []string{"a"},
		Objects: []*RequestVars{
			&RequestVars{},
		},
	}), "\n\n")
	fmt.Println(util.S2Json(&Representation{
		Actions: map[string]*link{
			"a": &link{
				Header: map[string]string{
					"a": "b",
				},
				ExpiresAt: time.Now(),
			},
		},
		Error: &ObjectError{},
	}), "\n\n")
}
