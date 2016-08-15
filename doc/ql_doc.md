## The RQL Select Command

    Select A1, A2, …, Am
    From R1, R2, …, Rn
    [Where A1’ comp1 AV1 And A2’ comp2 AV2 And … And Ak’ compk AVk];

This command has the standard SQL interpretation: The result of the command is structured as a relation with attributes A1, A2, …, Am. For each tuple t in the cross-product of relations R1, R2, …, Rn in the From clause, if tuple t satisfies all conditions in the Where clause, then the attributes of tuple t listed in the Select clause are included in the result (in the listed order). If the Where clause is omitted, it is considered to be true always. Duplicate tuples in the result are not eliminated.

An alternative form of the Select command is:

    Select *
    From R1, R2, …, Rn
    [Where A1’ comp1 AV1 And A2’ comp2 AV2 And … And Ak’ compk AVk];


## The RQL Delete Command

    Delete from R1
    [Where A1’ comp1 AV1 And A2’ comp2 AV2 And … And Ak’ compk AVk];
    
## The RQL Updata Command
    Update R1 set A1 = AV1 
    [Where A1’ comp1 AV1' And A2’ comp2 AV2' And … And Ak’ compk AVk'];
    
## The RQL Insert Command
    Insert into R1 values ( V1, V2, ..., Vk );
